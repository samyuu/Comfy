#include "WASAPIBackend.h"
#include "Core/Logger.h"
#include <atomic>

#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <avrt.h>

#pragma comment(lib, "avrt.lib")

using Microsoft::WRL::ComPtr;

namespace Comfy::Audio
{
	struct WASAPIBackend::Impl
	{
	public:
		bool OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback)
		{
			if (isOpenRunning)
				return false;

			streamParam = param;
			renderCallback = std::move(callback);

			HRESULT error = S_OK;
			error = ::CoInitialize(nullptr);

			error = ::CoCreateInstance(__uuidof(::MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(::IMMDeviceEnumerator), &deviceEnumerator);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to create MMDeviceEnumerator. Error: 0x%X", error);
				return false;
			}

			error = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to retrieve default audio endpoint. Error: 0x%X", error);
				return false;
			}

			error = device->Activate(__uuidof(::IAudioClient), CLSCTX_ALL, nullptr, &audioClient);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to activate audio client for device. Error: 0x%X", error);
				return false;
			}

			waveformat.wFormatTag = WAVE_FORMAT_PCM;
			waveformat.nChannels = streamParam.ChannelCount;
			waveformat.nSamplesPerSec = streamParam.SampleRate;
			waveformat.wBitsPerSample = sizeof(i16) * CHAR_BIT;
			waveformat.nBlockAlign = (waveformat.nChannels * waveformat.wBitsPerSample / CHAR_BIT);
			waveformat.nAvgBytesPerSec = (waveformat.nSamplesPerSec * waveformat.nBlockAlign);
			waveformat.cbSize = 0;

			constexpr auto sharedStreamFlags = (AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);
			constexpr auto exclusiveStreamFlags = (AUDCLNT_STREAMFLAGS_EVENTCALLBACK);

			const auto shareMode = (streamParam.Mode == StreamShareMode::Shared) ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;
			const auto streamFlags = (streamParam.Mode == StreamShareMode::Shared) ? sharedStreamFlags : exclusiveStreamFlags;

			// TODO: Account for user requested frame buffer size (?)
			//		 at least for shared mode where ~400 samples (~9ms) are longer than that of a 144FPS frame
			//		 for exclusive streams the native buffer size should be optimal (?)
			//		 need to "emulate" the requested interval manually (?)
			error = audioClient->GetDevicePeriod(nullptr, &bufferTimeDuration);

			deviceTimePeriod = (streamParam.Mode == StreamShareMode::Shared) ? 0 : bufferTimeDuration;

			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to retrieve device period. Error: 0x%X", error);
				return false;
			}

			error = audioClient->Initialize(shareMode, streamFlags, bufferTimeDuration, deviceTimePeriod, &waveformat, nullptr);

			if (error == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
			{
				error = audioClient->GetBufferSize(&bufferFrameCount);
				if (FAILED(error))
				{
					Logger::LogErrorLine(__FUNCTION__"(): Unable to get audio client buffer size. Error: 0x%X", error);
					return false;
				}

				bufferTimeDuration = static_cast<::REFERENCE_TIME>((10000.0 * 1000 / waveformat.nSamplesPerSec * bufferFrameCount) + 0.5);
				deviceTimePeriod = (streamParam.Mode == StreamShareMode::Shared) ? 0 : bufferTimeDuration;

				error = device->Activate(__uuidof(::IAudioClient), CLSCTX_ALL, nullptr, &audioClient);
				if (FAILED(error))
				{
					Logger::LogErrorLine(__FUNCTION__"(): Unable to activate audio client for device. Error: 0x%X", error);
					return false;
				}

				error = audioClient->Initialize(shareMode, streamFlags, bufferTimeDuration, deviceTimePeriod, &waveformat, nullptr);
				if (FAILED(error))
				{
					Logger::LogErrorLine(__FUNCTION__"(): Unable to initialize audio client. Error: 0x%X", error);
					return false;
				}
			}
			else if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to initialize audio client. Error: 0x%X", error);
				return false;
			}

			audioClientEvent = ::CreateEventW(nullptr, false, false, nullptr);
			if (audioClientEvent == NULL)
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to create audio client event. Error: 0x%X", ::GetLastError());
				return false;
			}

			error = audioClient->SetEventHandle(audioClientEvent);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to set audio client event handle. Error: 0x%X", error);
				return false;
			}

			error = audioClient->GetService(__uuidof(::IAudioRenderClient), &renderClient);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to get audio render client. Error: 0x%X", error);
				return false;
			}

			error = audioClient->GetService(__uuidof(::ISimpleAudioVolume), &simpleAudioVolume);
			if (FAILED(error))
				Logger::LogErrorLine(__FUNCTION__"(): Unable to get simple audio volume interface. Error: 0x%X", error);

			renderThread = ::CreateThread(nullptr, 0, [](LPVOID lpParameter) -> DWORD
			{
				return static_cast<DWORD>(reinterpret_cast<WASAPIBackend::Impl*>(lpParameter)->RenderThreadEntryPoint());
			}, static_cast<void*>(this), 0, nullptr);

			if (renderThread == NULL)
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to create render thread. Error: 0x%X", ::GetLastError());
				return false;
			}

			isOpenRunning = true;
			return true;
		}

		bool StopCloseStream()
		{
			if (!isOpenRunning)
				return false;

			isOpenRunning = false;
			renderThreadStopRequested = true;
			if (renderThread != NULL)
			{
				::WaitForSingleObject(renderThread, INFINITE);
				::CloseHandle(renderThread);
				renderThread = NULL;
			}
			renderThreadStopRequested = false;

			if (audioClientEvent != NULL)
			{
				::CloseHandle(audioClientEvent);
				audioClientEvent = NULL;
			}

			simpleAudioVolume = nullptr;
			renderClient = nullptr;
			audioClient = nullptr;
			device = nullptr;
			deviceEnumerator = nullptr;

			return true;
		}

	public:
		bool IsOpenRunning() const
		{
			return isOpenRunning;
		}

	public:
		u32 RenderThreadEntryPoint()
		{
			if (audioClient == nullptr || renderClient == nullptr)
			{
				Logger::LogErrorLine(__FUNCTION__"(): Audio client uninitialized");
				return -1;
			}

			HRESULT error = S_OK;
			error = ::CoInitialize(nullptr);

			error = audioClient->GetBufferSize(&bufferFrameCount);
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to get audio client buffer size. Error: 0x%X", error);
				return -1;
			}

			// NOTE: Load the first buffer with data before starting the stream to to reduce latency
			BYTE* tempOutputBuffer = nullptr;
			error = renderClient->GetBuffer(bufferFrameCount, &tempOutputBuffer);

			if (!FAILED(error))
				RenderThreadProcessOutputBuffer(reinterpret_cast<i16*>(tempOutputBuffer), bufferFrameCount, streamParam.ChannelCount);

			DWORD releaseBufferFlags = 0;
			error = renderClient->ReleaseBuffer(bufferFrameCount, releaseBufferFlags);

			// NOTE: Ask MMCSS to temporarily boost the thread priority to reduce glitches while the low-latency stream plays.
			proAudioTaskIndex = 0;
			proAudioTask = ::AvSetMmThreadCharacteristicsW(L"Pro Audio", &proAudioTaskIndex);
			if (proAudioTask == NULL)
				Logger::LogErrorLine(__FUNCTION__"(): Unable to assign Pro Audio thread characteristics to render thread. Error: 0x%X", error);

			error = audioClient->Start();
			if (FAILED(error))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to start audio client. Error: 0x%X", error);
				return -1;
			}

			while (releaseBufferFlags != AUDCLNT_BUFFERFLAGS_SILENT)
			{
				if (renderThreadStopRequested)
					break;

				const auto waitObjectResult = ::WaitForSingleObject(audioClientEvent, 2000);
				if (waitObjectResult != WAIT_OBJECT_0)
				{
					Logger::LogErrorLine(__FUNCTION__"(): Audio client event timeout. Error: 0x%X", ERROR_TIMEOUT);
					break;
				}

				error = audioClient->GetBufferSize(&bufferFrameCount);
				if (FAILED(error) || bufferFrameCount == 0)
					continue;

				UINT32 remainingFrameCount = bufferFrameCount;
				if (streamParam.Mode == StreamShareMode::Shared)
				{
					UINT32 paddingFrameCount;
					error = audioClient->GetCurrentPadding(&paddingFrameCount);
					if (!FAILED(error))
						remainingFrameCount -= paddingFrameCount;
				}

				error = renderClient->GetBuffer(remainingFrameCount, &tempOutputBuffer);

				if (!FAILED(error))
					RenderThreadProcessOutputBuffer(reinterpret_cast<i16*>(tempOutputBuffer), remainingFrameCount, streamParam.ChannelCount);

				error = renderClient->ReleaseBuffer(remainingFrameCount, releaseBufferFlags);
			}

			error = audioClient->Stop();
			if (FAILED(error))
				Logger::LogErrorLine(__FUNCTION__"(): Unable to stop audio client. Error: 0x%X", error);

			if (proAudioTask != NULL)
				::AvRevertMmThreadCharacteristics(proAudioTask);

			return 0;
		}

		void RenderThreadProcessOutputBuffer(i16* outputBuffer, const u32 frameCount, const u32 channelCount)
		{
			if (outputBuffer == nullptr)
				return;

			renderCallback(outputBuffer, frameCount, channelCount);

			auto i16ToF32 = [](i16 v) -> f32 { return static_cast<f32>(v) * static_cast<f32>(INT16_MAX); };
			auto f32ToI16 = [](f32 v) -> i16 { return static_cast<i16>(v / static_cast<f32>(INT16_MAX)); };

			if (applySharedSessionVolume && streamParam.Mode == StreamShareMode::Exclusive)
			{
				float sharedSessionVolume = 1.0f;
				const auto error = (simpleAudioVolume != nullptr) ? simpleAudioVolume->GetMasterVolume(&sharedSessionVolume) : E_POINTER;

				if (!FAILED(error) && sharedSessionVolume >= 0.0f && sharedSessionVolume < 1.0f)
				{
					for (u32 i = 0; i < (frameCount * streamParam.ChannelCount); i++)
						outputBuffer[i] = f32ToI16(i16ToF32(outputBuffer[i]) * sharedSessionVolume);
				}
			}
		}

	private:
		StreamParameters streamParam = {};
		RenderCallbackFunc renderCallback;

		std::atomic<bool> isOpenRunning = false;
		std::atomic<bool> renderThreadStopRequested = false;

		bool applySharedSessionVolume = true;

		::WAVEFORMATEX waveformat = {};
		::REFERENCE_TIME bufferTimeDuration = {}, deviceTimePeriod = {};
		::UINT32 bufferFrameCount = {};

		::HANDLE renderThread = NULL;

		::DWORD proAudioTaskIndex = {};
		::HANDLE proAudioTask = {};

		::HANDLE audioClientEvent = NULL;
		ComPtr<::IMMDeviceEnumerator> deviceEnumerator = nullptr;
		ComPtr<::IMMDevice> device = nullptr;
		ComPtr<::IAudioClient> audioClient = nullptr;
		ComPtr<::IAudioRenderClient> renderClient = nullptr;
		ComPtr<::ISimpleAudioVolume> simpleAudioVolume = nullptr;
	};

	WASAPIBackend::WASAPIBackend() : impl(std::make_unique<Impl>())
	{
	}

	WASAPIBackend::~WASAPIBackend()
	{
	}

	bool WASAPIBackend::OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback)
	{
		return impl->OpenStartStream(param, std::move(callback));
	}

	bool WASAPIBackend::StopCloseStream()
	{
		return impl->StopCloseStream();
	}

	bool WASAPIBackend::IsOpenRunning() const
	{
		return impl->IsOpenRunning();
	}
}
