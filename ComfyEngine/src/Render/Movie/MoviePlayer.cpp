#include <Windows.h>

#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#include "MoviePlayer.h"
#include "Render/D3D11/D3D11.h"
#include "Render/D3D11/D3D11Texture.h"
#include "Render/D3D11/D3D11OpaqueResource.h"
#include "Render/Core/Renderer2D/AetRenderer.h"
#include <atomic>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mfmediaengine.h>
#include <Mferror.h>
#include <propvarutil.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Propsys.lib")

namespace Comfy::Render
{
	namespace
	{
		struct BStrArg
		{
			BStrArg(std::string_view value) { BStr = ::SysAllocString(UTF8::WideArg(value).c_str()); }
			~BStrArg() { ::SysFreeString(BStr); }

			operator BSTR() const { return BStr; }
			BSTR BStr;
		};

		class MediaEngineNotifyFunctionWrapper : public IMFMediaEngineNotify
		{
		public:
			MediaEngineNotifyFunctionWrapper(std::function<HRESULT(DWORD, DWORD_PTR, DWORD)> func) : onEventNotify(func) { AddRef(); }
			virtual ~MediaEngineNotifyFunctionWrapper() { Release(); }

		public:
			// IUnknown
			STDMETHODIMP_(ULONG) AddRef() override
			{
				return InterlockedIncrement(&refCount);
			}

			STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv) override
			{
				if (ppv == nullptr)
					return E_POINTER;

				if (iid == __uuidof(IUnknown)) { *ppv = static_cast<IUnknown*>(this); }
				else if (iid == __uuidof(IMFMediaEngineNotify)) { *ppv = static_cast<IMFMediaEngineNotify*>(this); }
				else { *ppv = nullptr; return E_NOINTERFACE; }

				AddRef();
				return S_OK;
			}

			STDMETHODIMP_(ULONG) Release() override
			{
				const auto remainingRefCount = InterlockedDecrement(&refCount);

				if (remainingRefCount == 0)
					delete this;

				return remainingRefCount;
			}

			// IMFMediaEngineNotify
			HRESULT STDMETHODCALLTYPE EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) override
			{
				return onEventNotify(event, param1, param2);
			}

		private:
			std::function<HRESULT(DWORD, DWORD_PTR, DWORD)> onEventNotify;
			long refCount = 0;
		};

		class MemoryByteStreamReadRequest : public IUnknown
		{
		public:
			MemoryByteStreamReadRequest(ULONG bytesRead) : bytesRead(bytesRead) { AddRef(); }
			virtual ~MemoryByteStreamReadRequest() { Release(); }

			inline ULONG GetBytesRead() const { return bytesRead; }

			// IUnknown
			STDMETHODIMP_(ULONG) AddRef() override
			{
				return InterlockedIncrement(&refCount);
			}

			STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv) override
			{
				if (ppv == nullptr)
					return E_POINTER;

				if (iid == __uuidof(IUnknown)) { *ppv = static_cast<IUnknown*>(this); }
				else { *ppv = nullptr; return E_NOINTERFACE; }

				AddRef();
				return S_OK;
			}

			STDMETHODIMP_(ULONG) Release() override
			{
				const auto remainingRefCount = InterlockedDecrement(&refCount);

				if (remainingRefCount == 0)
					delete this;

				return remainingRefCount;
			}

		private:
			ULONG bytesRead = 0;
			long refCount = 0;
		};

		class ContiniousMemoryBufferMFByteStream : public IMFByteStream
		{
		public:
			ContiniousMemoryBufferMFByteStream(std::unique_ptr<u8[]> data, size_t dataSize) : streamData(std::move(data)), streamSize(dataSize) { AddRef(); }
			virtual ~ContiniousMemoryBufferMFByteStream() { Release(); }

		public:
			// IUnknown
			STDMETHODIMP_(ULONG) AddRef() override
			{
				return InterlockedIncrement(&refCount);
			}

			STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv) override
			{
				if (ppv == nullptr)
					return E_POINTER;

				if (iid == __uuidof(IUnknown)) { *ppv = static_cast<IUnknown*>(this); }
				else if (iid == __uuidof(IMFByteStream)) { *ppv = static_cast<IMFByteStream*>(this); }
				else { *ppv = nullptr; return E_NOINTERFACE; }

				AddRef();
				return S_OK;
			}

			STDMETHODIMP_(ULONG) Release() override
			{
				const auto remainingRefCount = InterlockedDecrement(&refCount);

				if (remainingRefCount == 0)
					delete this;

				return remainingRefCount;
			}

			// IMFByteStream
			HRESULT STDMETHODCALLTYPE GetCapabilities(__RPC__out DWORD* pdwCapabilities) override
			{
				if (pdwCapabilities == nullptr)
					return E_POINTER;

				*pdwCapabilities |= MFBYTESTREAM_IS_READABLE;
				*pdwCapabilities |= MFBYTESTREAM_IS_SEEKABLE;
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE GetLength(__RPC__out QWORD* pqwLength) override
			{
				if (pqwLength == nullptr)
					return E_POINTER;

				*pqwLength = streamSize;
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE SetLength(QWORD qwLength) override { return E_NOTIMPL; }

			HRESULT STDMETHODCALLTYPE GetCurrentPosition(__RPC__out QWORD* pqwPosition) override
			{
				if (pqwPosition == nullptr)
					return E_POINTER;

				*pqwPosition = currentPosition;
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD qwPosition) override
			{
				currentPosition = Clamp(0, streamSize, qwPosition);
				return (qwPosition > streamSize) ? E_INVALIDARG : S_OK;
			}

			HRESULT STDMETHODCALLTYPE IsEndOfStream(__RPC__out BOOL* pfEndOfStream) override
			{
				if (pfEndOfStream == nullptr)
					return E_POINTER;

				*pfEndOfStream = (currentPosition >= streamSize);
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE Read(__RPC__out_ecount_full(cb) BYTE* pb, ULONG cb, __RPC__out ULONG* pcbRead) override
			{
				if (pb == nullptr)
					return E_POINTER;

				const QWORD availableBytesToRead = Clamp(0, GetRemainingBytes(), cb);
				if (availableBytesToRead > 0)
				{
					std::memcpy(pb, &streamData[currentPosition], availableBytesToRead);
					currentPosition += availableBytesToRead;
				}

				if (pcbRead != nullptr)
					*pcbRead = static_cast<ULONG>(availableBytesToRead);

				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE BeginRead(_Out_writes_bytes_(cb) BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override
			{
				HRESULT hr = S_OK;
				isAsyncReading = true;

				ULONG bytesRead = {};
				hr = Read(pb, cb, &bytesRead);

				ComPtr<MemoryByteStreamReadRequest> readRequest;
				readRequest.Attach(new MemoryByteStreamReadRequest(bytesRead));

				IMFAsyncResult* result = nullptr;
				hr = ::MFCreateAsyncResult(readRequest.Get(), pCallback, punkState, &result);

				if (FAILED(hr))
					return hr;

				result->SetStatus(S_OK);
				hr = ::MFInvokeCallback(result);

				return hr;
			}

			HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult* pResult, _Out_  ULONG* pcbRead) override
			{
				HRESULT hr = S_OK;

				IUnknown* readRequestIUnknown = nullptr;
				hr = pResult->GetObjectA(&readRequestIUnknown);

				if (readRequestIUnknown == nullptr)
					return E_INVALIDARG;

				ComPtr<MemoryByteStreamReadRequest> readRequest;
				readRequest.Attach(static_cast<MemoryByteStreamReadRequest*>(readRequestIUnknown));
				*pcbRead = readRequest->GetBytesRead();

				isAsyncReading = false;

				hr = pResult->GetStatus();
				pResult->Release();

				assert(SUCCEEDED(hr));
				return hr;
			}

			HRESULT STDMETHODCALLTYPE Write(__RPC__in_ecount_full(cb) const BYTE* pb, ULONG cb, __RPC__out ULONG* pcbWritten) override { return E_NOTIMPL; }

			HRESULT STDMETHODCALLTYPE BeginWrite(_In_reads_bytes_(cb) const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override { return E_NOTIMPL; }

			HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult* pResult, _Out_ ULONG* pcbWritten) override { return E_NOTIMPL; }

			HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, __RPC__out QWORD* pqwCurrentPosition) override
			{
				if (SeekOrigin == msoBegin)
					currentPosition = Clamp(0, streamSize, llSeekOffset);
				else if (SeekOrigin == msoCurrent)
					currentPosition = Clamp(0, streamSize, currentPosition + llSeekOffset);
				else
					return E_INVALIDARG;

				if (pqwCurrentPosition != nullptr)
					*pqwCurrentPosition = currentPosition;

				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE Flush(void) override { return S_OK; }

			virtual HRESULT STDMETHODCALLTYPE Close(void) override { return S_OK; }

		private:
			inline QWORD GetRemainingBytes() const { return (currentPosition >= streamSize) ? 0 : (streamSize - currentPosition); }

			static constexpr QWORD Clamp(QWORD min, QWORD max, QWORD value) { return ((max < value) ? max : (value < min) ? min : value); }

		private:
			long refCount = 0;

			std::atomic<bool> isAsyncReading = false;

			std::unique_ptr<u8[]> streamData;
			QWORD streamSize = 0;
			QWORD currentPosition = 0;
		};

		struct D3D11MovieFrameTextureData
		{
			Graphics::Tex Tex;
			Graphics::Spr Spr;
			D3D11_TEXTURE2D_DESC Desc;
		};

		HRESULT QueryMediaEngineExStreamAttributes(IMFMediaEngineEx& mediaEngineEx, MoviePlayerStreamAttributes& outAttributes)
		{
			HRESULT hr = S_OK;
			PROPVARIANT propVariant = {};

			{
				hr = mediaEngineEx.GetPresentationAttribute(MF_PD_DURATION, &propVariant);
				outAttributes.PresentationDurationMFTime = propVariant.hVal.QuadPart;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetPresentationAttribute(MF_PD_LAST_MODIFIED_TIME, &propVariant);
				outAttributes.LastModifiedFileTime = propVariant.hVal.QuadPart;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetPresentationAttribute(MF_PD_TOTAL_FILE_SIZE, &propVariant);
				outAttributes.TotalFileSize = propVariant.hVal.QuadPart;
				hr = ::PropVariantClear(&propVariant);
			}

			DWORD streamCount = {};
			hr = mediaEngineEx.GetNumberOfStreams(&streamCount);

			DWORD audioStreamIndex = streamCount;
			DWORD videoStreamIndex = streamCount;

			for (DWORD streamIndex = 0; streamIndex < streamCount; streamIndex++)
			{
				hr = mediaEngineEx.GetStreamAttribute(streamIndex, MF_MT_MAJOR_TYPE, &propVariant);

				GUID majorTypeGUID = {};
				hr = ::PropVariantToGUID(propVariant, &majorTypeGUID);
				hr = ::PropVariantClear(&propVariant);

				// NOTE: Assuming the first stream of each kind to be the only important one as for now that should cover 99.9% of cases
				if (majorTypeGUID == MFMediaType_Audio && audioStreamIndex == streamCount)
					audioStreamIndex = streamIndex;
				else if (majorTypeGUID == MFMediaType_Video && videoStreamIndex == streamCount)
					videoStreamIndex = streamIndex;
			}

			if (audioStreamIndex < streamCount)
			{
				outAttributes.HasAudioStream = true;

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &propVariant);
				outAttributes.Audio.AverageBytesPerSecond = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_BITS_PER_SAMPLE, &propVariant);
				outAttributes.Audio.BitsPerSample = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_BLOCK_ALIGNMENT, &propVariant);
				outAttributes.Audio.BlockAlignment = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, &propVariant);
				outAttributes.Audio.FloatSamplesPerSecond = propVariant.dblVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_NUM_CHANNELS, &propVariant);
				outAttributes.Audio.ChannelCount = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_SAMPLES_PER_BLOCK, &propVariant);
				outAttributes.Audio.SamplesPerBlock = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(audioStreamIndex, MF_MT_AUDIO_SAMPLES_PER_SECOND, &propVariant);
				outAttributes.Audio.SamplesPerSecond = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);
			}

			if (videoStreamIndex < streamCount)
			{
				outAttributes.HasVideoStream = true;

				hr = mediaEngineEx.GetStreamAttribute(videoStreamIndex, MF_MT_AVG_BITRATE, &propVariant);
				outAttributes.Video.AverageBitRate = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(videoStreamIndex, MF_MT_FRAME_RATE, &propVariant);
				outAttributes.Video.FrameRateNumerator = propVariant.hVal.HighPart;
				outAttributes.Video.FrameRateDenominator = propVariant.hVal.LowPart;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(videoStreamIndex, MF_MT_FRAME_SIZE, &propVariant);
				outAttributes.Video.FrameSizeWidth = propVariant.hVal.HighPart;
				outAttributes.Video.FrameSizeHeight = propVariant.hVal.LowPart;
				hr = ::PropVariantClear(&propVariant);

				hr = mediaEngineEx.GetStreamAttribute(videoStreamIndex, MF_MT_MAX_KEYFRAME_SPACING, &propVariant);
				outAttributes.Video.MaxKeyFrameSpacing = propVariant.uintVal;
				hr = ::PropVariantClear(&propVariant);
			}

			return hr;
		}
	}

	class D3D11MediaFoundationMediaEngineMoviePlayer : public IMoviePlayer
	{
	public:
		D3D11MediaFoundationMediaEngineMoviePlayer(D3D11& d3d11) : d3d11(d3d11)
		{
		}

		~D3D11MediaFoundationMediaEngineMoviePlayer() override
		{
			HRESULT hr = S_OK;

			if (mediaEngine != nullptr)
				hr = mediaEngine->Shutdown();

			// NOTE: Manual deletion just in case to ensure correct deletion order
			mediaEngineEx = nullptr;
			mediaEngine = nullptr;
			entireFileContentMemoryByteStream = nullptr;
			mediaEngineNotifyFunctionWrapper = nullptr;
		}

	public:
		HRESULT Initialize(DWORD mediaEngineCreationFlags = MF_MEDIA_ENGINE_REAL_TIME_MODE | MF_MEDIA_ENGINE_FORCEMUTE)
		{
			if (wasInitialized)
				return S_OK;

			HRESULT hr = S_OK;
			hr = InitializeGlobalD3D11StateOnce(d3d11);

			mediaEngineNotifyFunctionWrapper.Attach(new MediaEngineNotifyFunctionWrapper([&](DWORD event, DWORD_PTR param1, DWORD param2) -> HRESULT
			{
				return OnMediaEngineAsyncCallback(event, param1, param2);
			}));

			ComPtr<IMFMediaEngineClassFactory> mediaEngineFactory = nullptr;
			hr = ::CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(mediaEngineFactory), &mediaEngineFactory);

			if (mediaEngineFactory != nullptr)
			{
				ComPtr<IMFAttributes> mediaEngineCreationAttributes = nullptr;
				hr = ::MFCreateAttributes(&mediaEngineCreationAttributes, 4);
				if (mediaEngineCreationAttributes != nullptr)
				{
					hr = mediaEngineCreationAttributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, textureFormat);
					hr = mediaEngineCreationAttributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, mediaEngineNotifyFunctionWrapper.Get());
					hr = mediaEngineCreationAttributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, global.DXGIDeviceManager.Get());
				}

				hr = mediaEngineFactory->CreateInstance(mediaEngineCreationFlags, mediaEngineCreationAttributes.Get(), &mediaEngine);

				if (mediaEngine != nullptr)
				{
					hr = mediaEngine->QueryInterface(__uuidof(mediaEngineEx), &mediaEngineEx);
					hr = mediaEngine->SetMuted(true);
					hr = mediaEngine->SetVolume(0.0);
				}
			}

			wasInitialized = true;
			return hr;
		}

		HRESULT InitializeGlobalD3D11StateOnce(D3D11& d3d11)
		{
			if (!global.MediaFoundationInitialized)
			{
				// BUG: Calling CoInitializeEx() here makes IFileDialog::Show() lock up..?
				global.CoInitializeResult = ::CoInitialize(nullptr);
				global.MFStartupResult = ::MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
				global.MediaFoundationInitialized = true;
			}

			HRESULT hr = S_OK;

			if (!global.D3D11DeviceInitialized)
			{
				ComPtr<ID3D10Multithread> d3d10Multithread = nullptr;
				hr = d3d11.Device->QueryInterface(__uuidof(d3d10Multithread), &d3d10Multithread);
				if (d3d10Multithread != nullptr)
					hr = d3d10Multithread->SetMultithreadProtected(true);

				hr = ::MFCreateDXGIDeviceManager(&global.DXGIDeviceManagerID, &global.DXGIDeviceManager);
				if (global.DXGIDeviceManager != nullptr)
					hr = global.DXGIDeviceManager->ResetDevice(d3d11.Device.Get(), global.DXGIDeviceManagerID);

				global.D3D11DeviceInitialized = true;
			}

			return hr;
		}

	public:
		bool OpenFileAsync(std::string_view filePath) override
		{
			return InternalOpenFileOrOpenBytesOrCloseFileAsync(filePath, nullptr, 0);
		}

		bool OpenFileBytesAsync(std::string_view fileName, std::unique_ptr<u8[]> fileBytes, size_t fileSize) override
		{
			return InternalOpenFileOrOpenBytesOrCloseFileAsync(fileName, std::move(fileBytes), fileSize);
		}

		bool CloseFileAsync() override
		{
			return InternalOpenFileOrOpenBytesOrCloseFileAsync("", nullptr, 0);
		}

		std::string GetFilePath() const override
		{
			return videoFilePath;
		}

		bool GetIsLoadingFileAsync() const override
		{
			// BUG: Looks like this isn't always accurate..?
			return (mediaEngine != nullptr && atomic.CurrentlyLoadingMetadata.load());
		}

		void WaitUntilFileOpenCompletedSync(TimeSpan timeoutDuration) override
		{
			// HACK:
			const auto startTime = TimeSpan::GetTimeNow();
			while (atomic.CurrentlyLoadingMetadata)
			{
				if ((TimeSpan::GetTimeNow() - startTime) >= timeoutDuration)
					break;
				::Sleep(1);
			}
		}

		bool GetIsPlaying() const override
		{
			return (mediaEngine == nullptr) ? false : (mediaEngine->HasVideo() && !mediaEngine->IsPaused() && !mediaEngine->IsEnded());
		}

		bool SetIsPlayingAsync(bool value) override
		{
			if (mediaEngine == nullptr)
				return false;

			if (mediaEngine->IsEnded())
				return false;

			bool wasSuccessful = false;
			if (value)
			{
				wasSuccessful = SUCCEEDED(mediaEngine->Play());
				if (const f32 speed = atomic.PrePausePlaybackSpeed.load(); speed != 1.0f)
					mediaEngine->SetPlaybackRate(static_cast<f64>(speed));
			}
			else
			{
				atomic.PrePausePlaybackSpeed = static_cast<f32>(mediaEngine->GetPlaybackRate());
				wasSuccessful = SUCCEEDED(mediaEngine->Pause());
			}

			return wasSuccessful;
		}

		bool GetIsSeeking() const override
		{
			return (mediaEngine != nullptr) ? mediaEngine->IsSeeking() : false;
		}

		f32 GetPlaybackSpeed() const override
		{
			return (mediaEngine != nullptr) ? static_cast<f32>(mediaEngine->GetPlaybackRate()) : 0.0f;
		}

		bool SetPlaybackSpeedAsync(f32 value) override
		{
			return (mediaEngine != nullptr) ? SUCCEEDED(mediaEngine->SetPlaybackRate(static_cast<f32>(value))) : false;
		}

		bool GetIsScrubbing() const override
		{
			return (mediaEngine != nullptr) ? atomic.IsScrubbing.load() : false;
		}

		bool SetIsScrubbingAsync(bool value) override
		{
			if (mediaEngine == nullptr || atomic.IsScrubbing.exchange(value) == value)
				return false;

			bool wasSuccessful = false;
			if (value)
			{
				atomic.PreScrubbingPlaybackSpeed = static_cast<f32>(mediaEngine->GetPlaybackRate());
				wasSuccessful = SUCCEEDED(mediaEngine->SetPlaybackRate(0.0));
			}
			else
			{
				wasSuccessful = SUCCEEDED(mediaEngine->SetPlaybackRate(static_cast<f64>(atomic.PreScrubbingPlaybackSpeed)));
				atomic.PreScrubbingPlaybackSpeed = {};
			}

			return wasSuccessful;
		}

		bool GetIsPlaybackSpeedSupported(f32 value) override
		{
			return (mediaEngineEx != nullptr) ? mediaEngineEx->IsPlaybackRateSupported(value) : false;
		}

		TimeSpan GetPosition() const override
		{
			return (mediaEngine != nullptr) ? TimeSpan::FromSeconds(mediaEngine->GetCurrentTime()) : TimeSpan::Zero();
		}

		bool SetPositionAsync(TimeSpan value, bool accurate) override
		{
			const auto duration = GetDuration();
			value = (value <= TimeSpan::Zero()) ? TimeSpan::Zero() : (value >= duration) ? duration : value;

			if (mediaEngineEx != nullptr)
				return SUCCEEDED(mediaEngineEx->SetCurrentTimeEx(value.TotalSeconds(), accurate ? MF_MEDIA_ENGINE_SEEK_MODE_NORMAL : MF_MEDIA_ENGINE_SEEK_MODE_APPROXIMATE));
			else if (mediaEngine != nullptr)
				return SUCCEEDED(mediaEngine->SetCurrentTime(value.TotalSeconds()));
			else
				return false;
		}

		bool FrameStepAsync(bool forward) override
		{
			return (mediaEngineEx != nullptr) ? SUCCEEDED(mediaEngineEx->FrameStep(forward)) : false;
		}

		TimeSpan GetDuration() const override
		{
			if (mediaEngine == nullptr)
				return TimeSpan::Zero();

			// NOTE: Returns NaN if no media data is available and INF if the duration is unbounded
			const f64 durationSec = mediaEngine->GetDuration();
			return (std::isnan(durationSec)) ? TimeSpan::Zero() : TimeSpan::FromSeconds(durationSec);
		}

		ivec2 GetResolution() const override
		{
			return (mediaEngine != nullptr) ? atomic.VideoResolution.load() : ivec2 { 0, 0 };
		}

		bool GetIsLooping() const override
		{
			return (mediaEngine != nullptr) ? mediaEngine->GetLoop() : false;
		}

		bool SetIsLoopingAsync(bool value) override
		{
			return (mediaEngine != nullptr) ? SUCCEEDED(mediaEngine->SetLoop(value)) : false;
		}

		bool GetHasVideoStream() const override
		{
			return (mediaEngine != nullptr) ? mediaEngine->HasVideo() : false;
		}

		bool GetHasAudioStream() const override
		{
			return (mediaEngine != nullptr) ? mediaEngine->HasAudio() : false;
		}

		bool RegisterAsyncCallback(MoviePlayerAsyncCallbackFunc callbackFunc) override
		{
			if (mediaEngine == nullptr)
				return false;

			userAsyncCallbackFunc = std::move(callbackFunc);
			return true;
		}

		bool TryGetStreamAttributes(MoviePlayerStreamAttributes& outAttributes) const override
		{
			outAttributes = {};
			if (mediaEngineEx == nullptr)
				return false;

			outAttributes = atomic.StreamAttributes.load();
			return true;
		}

		ComfyTextureID GetCurrentTexture() override
		{
			const D3D11MovieFrameTextureData* movieVideoFrame = InternalGetAndTransferCurrentVideoFrame();
			return (movieVideoFrame != nullptr) ? ComfyTextureID(GetD3D11Texture2DView(d3d11, movieVideoFrame->Tex)) : nullptr;
		}

		TexSprView GetCurrentTextureAsTexSprView() override
		{
			const D3D11MovieFrameTextureData* movieVideoFrame = InternalGetAndTransferCurrentVideoFrame();
			return (movieVideoFrame != nullptr) ? TexSprView { &movieVideoFrame->Tex, &movieVideoFrame->Spr } : TexSprView { nullptr, nullptr };
		}

	private:
		bool InternalOpenFileOrOpenBytesOrCloseFileAsync(std::string_view filePathOrName, std::unique_ptr<u8[]> optionalfileBytes, size_t optionalFileSize)
		{
			videoFilePath = filePathOrName;
			entireFileContentMemoryByteStream.Reset();

			atomic.IsScrubbing = false;
			atomic.PreScrubbingPlaybackSpeed = 1.0f;
			atomic.PrePausePlaybackSpeed = 1.0f;
			atomic.FrameTextureBufferContentInvalidated = true;
			atomic.VideoResolution = ivec2 {};
			atomic.StreamAttributes = MoviePlayerStreamAttributes {};

			// NOTE: To make extra sure old frames are never shown. changing the media source shouldn't happen frequently anyway
			frameTextureBuffer = {};

			if (optionalfileBytes != nullptr)
			{
				if (mediaEngineEx == nullptr)
					return false;

				entireFileContentMemoryByteStream.Attach(new ContiniousMemoryBufferMFByteStream(std::move(optionalfileBytes), optionalFileSize));

				atomic.CurrentlyLoadingMetadata = true;
				return SUCCEEDED(mediaEngineEx->SetSourceFromByteStream(entireFileContentMemoryByteStream.Get(), BStrArg(filePathOrName)));
			}
			else
			{
				if (mediaEngine == nullptr)
					return false;

				atomic.CurrentlyLoadingMetadata = true;
				return SUCCEEDED(filePathOrName.empty() ? mediaEngine->SetSource(nullptr) : mediaEngine->SetSource(BStrArg(filePathOrName)));
			}
		}

		D3D11MovieFrameTextureData* InternalGetAndTransferCurrentVideoFrame()
		{
			if (mediaEngine == nullptr || !mediaEngine->HasVideo())
				return nullptr;

			HRESULT hr = S_OK;

			const ivec2 videoResolution = atomic.VideoResolution.load();
			const i32 videoWidth = videoResolution.x;
			const i32 videoHeight = videoResolution.y;
			if (videoWidth <= 0 || videoHeight <= 0)
				return nullptr;

			if (frameTextureBuffer[0].Desc.Width != videoWidth || frameTextureBuffer[0].Desc.Height != videoHeight)
			{
				for (size_t frameTextureIndex = 0; frameTextureIndex < frameTextureBuffer.size(); frameTextureIndex++)
				{
					auto& frameTexture = frameTextureBuffer[frameTextureIndex];
					frameTexture.Desc.Width = videoWidth;
					frameTexture.Desc.Height = videoHeight;
					frameTexture.Desc.MipLevels = 1;
					frameTexture.Desc.ArraySize = 1;
					frameTexture.Desc.Format = textureFormat;
					frameTexture.Desc.SampleDesc.Count = 1;
					frameTexture.Desc.Usage = D3D11_USAGE_DEFAULT;
					frameTexture.Desc.BindFlags = (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

					// HACK: Abusing the fact that the D3D11OpaqueResource accessor functions won't try to upload the Tex CPU data if the unique_ptr resource is already initialized.
					//		 Wrapping this inside Tex and Spr for now so that they can easily be used with exiting Renderer2D / AetRenderer code.
					//		 Instead of doing this, the entire Renderer2D interface should probably be changed to not operate on the Graphics types directly
					//		 to make internop with regular D3D11 code like this easier.
					{
						auto& baseMipMap = frameTexture.Tex.MipMapsArray.empty() ? frameTexture.Tex.MipMapsArray.emplace_back().emplace_back() : frameTexture.Tex.MipMapsArray[0][0];
						baseMipMap.Size = videoResolution;
						baseMipMap.Format = Graphics::TextureFormat::RGBA8;
						baseMipMap.Data = nullptr;
						baseMipMap.DataSize = 0;

						frameTexture.Tex.GPU_Texture2D.Resource = nullptr;
						frameTexture.Tex.GPU_Texture2D.Resource = std::make_unique<D3D11Texture2DAndView>(d3d11, frameTexture.Desc);
						frameTexture.Tex.GPU_FlipY = true;

						frameTexture.Spr.TextureIndex = 0;
						frameTexture.Spr.Rotate = 0;
						frameTexture.Spr.TexelRegion = vec4(0.0f, 0.0f, 1.0f, 1.0f);
						frameTexture.Spr.PixelRegion = vec4(0.0f, 0.0f, static_cast<f32>(videoWidth), static_cast<f32>(videoHeight));
						frameTexture.Spr.Extra.ScreenMode = Graphics::ScreenMode::HDTV1080;

						D3D11_SetObjectDebugName(GetD3D11Texture2D(d3d11, frameTexture.Tex)->Texture.Get(), "MoviePlayer: FrameTextureBuffer[%zu]", frameTextureIndex);
					}
				}
			}

			LONGLONG videoStreamTickPresentationTime = {};
			hr = mediaEngine->OnVideoStreamTick(&videoStreamTickPresentationTime);
			if (hr == S_OK)
			{
				textureBufferIndexLast = textureBufferIndex;
				textureBufferIndex = (++textureBufferIndex >= static_cast<i32>(frameTextureBuffer.size())) ? 0 : textureBufferIndex;
				atomic.FrameTextureBufferContentInvalidated = false;

				D3D11Texture2DAndView* d3d11TextureAndView = GetD3D11Texture2D(d3d11, frameTextureBuffer[textureBufferIndex].Tex);
				assert(d3d11TextureAndView != nullptr && d3d11TextureAndView->Texture != nullptr);

				hr = mediaEngine->TransferVideoFrame(
					d3d11TextureAndView->Texture.Get(),
					nullptr,
					PtrArg(RECT { 0, 0, static_cast<LONG>(videoWidth), static_cast<LONG>(videoHeight) }),
					nullptr);

				return &frameTextureBuffer[textureBufferIndex];
			}
			else if (!atomic.FrameTextureBufferContentInvalidated.exchange(false))
			{
				return &frameTextureBuffer[textureBufferIndex];
			}
			else
			{
				return nullptr;
			}
		}

		HRESULT InternalDeselectAllAudioStreams()
		{
			if (mediaEngineEx == nullptr)
				return E_INVALIDARG;

			HRESULT hr = S_OK;

			DWORD streamCount = {};
			hr = mediaEngineEx->GetNumberOfStreams(&streamCount);
			if (FAILED(hr))
				return hr;

			for (DWORD i = 0; i < streamCount; i++)
			{
				PROPVARIANT propVariant = {};
				hr = mediaEngineEx->GetStreamAttribute(i, MF_MT_MAJOR_TYPE, &propVariant);

				GUID majorTypeGUID = {};
				hr = ::PropVariantToGUID(propVariant, &majorTypeGUID);
				hr = ::PropVariantClear(&propVariant);

				if (majorTypeGUID == MFMediaType_Audio)
					mediaEngineEx->SetStreamSelection(i, false);
			}

			hr = mediaEngineEx->ApplyStreamSelections();

			return hr;
		}

		HRESULT OnMediaEngineAsyncCallback(DWORD event, DWORD_PTR param1, DWORD param2)
		{
			MoviePlayerAsyncCallbackParam userCallbackParam =
			{
				this,
				MoviePlayerAsyncCallbackEvent::Unknown,
				MoviePlayerAsyncCallbackParam::EventUnionData { {}, },
			};

			switch (static_cast<MF_MEDIA_ENGINE_EVENT>(event))
			{
				// NOTE: The Media Engine has started to load the source
			case MF_MEDIA_ENGINE_EVENT_LOADSTART:
			{
				atomic.CurrentlyLoadingMetadata = true;
				InternalDeselectAllAudioStreams();
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadStarted;
			} break;
			// NOTE: The Media Engine is loading the source
			case MF_MEDIA_ENGINE_EVENT_PROGRESS:
			{
				InternalDeselectAllAudioStreams();
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadInProgress;
			} break;
			// NOTE: The Media Engine has suspended a load operation
			case MF_MEDIA_ENGINE_EVENT_SUSPEND:
			{
				atomic.CurrentlyLoadingMetadata = false;
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadSuspended;
			} break;
			// NOTE: The Media Engine cancelled a load operation that was in progress
			case MF_MEDIA_ENGINE_EVENT_ABORT:
			{
				atomic.CurrentlyLoadingMetadata = false;
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadAborted;
			} break;
			// NOTE: An error occurred
			case MF_MEDIA_ENGINE_EVENT_ERROR:
			{
				atomic.CurrentlyLoadingMetadata = false;
				atomic.LastMediaEngineError = static_cast<MF_MEDIA_ENGINE_ERR>(param1);
				atomic.LastMediaEngineHResult = static_cast<HRESULT>(param2);

				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::Error;
			} break;
			// NOTE: The Media Engine has switched to the MF_MEDIA_ENGINE_NETWORK_EMPTY state. 
			//		 This can occur when the IMFMediaEngine::Load method is called, or if an error occurs during the Load method
			case MF_MEDIA_ENGINE_EVENT_EMPTIED:
			{
				// ...
			} break;
			// NOTE: The Load algorithm is stalled, waiting for data
			case MF_MEDIA_ENGINE_EVENT_STALLED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadStalled;
			} break;
			// NOTE: The Media Engine is switching to the playing state
			case MF_MEDIA_ENGINE_EVENT_PLAY:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackSwitchedToPlay;
			} break;
			// NOTE: The media engine has paused
			case MF_MEDIA_ENGINE_EVENT_PAUSE:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackSwitchedToPause;
			} break;
			// NOTE: The Media Engine has loaded enough source data to determine the duration and dimensions of the source
			case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
			{
				DWORD nativeWidth = 0, nativeHeight = 0;
				HRESULT hr = mediaEngine->GetNativeVideoSize(&nativeWidth, &nativeHeight);

				atomic.VideoResolution = { static_cast<i32>(nativeWidth), static_cast<i32>(nativeHeight) };

				if (mediaEngineEx != nullptr)
				{
					MoviePlayerStreamAttributes streamAttributes = {};
					hr = QueryMediaEngineExStreamAttributes(*mediaEngineEx.Get(), streamAttributes);
					atomic.StreamAttributes = streamAttributes;

					InternalDeselectAllAudioStreams();
				}

				atomic.CurrentlyLoadingMetadata = false;
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadedMetadata;
			} break;
			// NOTE: The Media Engine has loaded enough data to render some content (for example, a video frame)
			case MF_MEDIA_ENGINE_EVENT_LOADEDDATA:
			{
				InternalDeselectAllAudioStreams();
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::LoadedData;
			} break;
			// NOTE: Playback has stopped because the next frame is not available
			case MF_MEDIA_ENGINE_EVENT_WAITING:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackPausedWaitingForNextFrame;
			} break;
			// NOTE: Playback has started
			case MF_MEDIA_ENGINE_EVENT_PLAYING:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackStarted;
			} break;
			// NOTE: Playback can start, but the Media Engine might need to stop to buffer more data
			case MF_MEDIA_ENGINE_EVENT_CANPLAY:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackCanPlay;
			} break;
			// NOTE: The Media Engine can probably play through to the end of the resource, without stopping to buffer data
			case MF_MEDIA_ENGINE_EVENT_CANPLAYTHROUGH:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackCanPlayThroughToEnd;
			} break;
			// NOTE: The Media Engine has started seeking to a new playback position
			case MF_MEDIA_ENGINE_EVENT_SEEKING:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackSeekStart;
			} break;
			// NOTE: The Media Engine has seeked to a new playback position
			case MF_MEDIA_ENGINE_EVENT_SEEKED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackSeekEnd;
			} break;
			// NOTE: The playback position has changed
			case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackPositionChanged;
				userCallbackParam.EventData.PlaybackPositionChanged.NewTime = TimeSpan::FromSeconds(mediaEngine->GetCurrentTime());
			} break;
			// NOTE: Playback has reached the end of the source. This event is not sent if the GetLoop is TRUE
			case MF_MEDIA_ENGINE_EVENT_ENDED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackReachedEnd;
			} break;
			// NOTE: The playback rate has changed
			case MF_MEDIA_ENGINE_EVENT_RATECHANGE:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackSpeedChanged;
				userCallbackParam.EventData.PlaybackSpeedChanged.NewSpeed = static_cast<f32>(mediaEngine->GetPlaybackRate());
			} break;
			// NOTE: The duration of the media source has changed
			case MF_MEDIA_ENGINE_EVENT_DURATIONCHANGE:
			{
				// ...
			} break;
			// NOTE: The audio volume changed
			case MF_MEDIA_ENGINE_EVENT_VOLUMECHANGE:
			{
				// ...
			} break;
			// NOTE: The output format of the media source has changed
			case MF_MEDIA_ENGINE_EVENT_FORMATCHANGE:
			{
				// ...
			} break;
			// NOTE: The Media Engine flushed any pending events from its queue
			case MF_MEDIA_ENGINE_EVENT_PURGEQUEUEDEVENTS:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::EventQueueCleared;
			} break;
			// NOTE: The playback position reached a timeline marker
			case MF_MEDIA_ENGINE_EVENT_TIMELINE_MARKER:
			{
				// ...
			} break;
			// NOTE: The audio balance changed
			case MF_MEDIA_ENGINE_EVENT_BALANCECHANGE:
			{
				// ...
			} break;
			// NOTE: The Media Engine has finished downloading the source data
			case MF_MEDIA_ENGINE_EVENT_DOWNLOADCOMPLETE:
			{
				// ...
			} break;
			// NOTE: The media source has started to buffer data
			case MF_MEDIA_ENGINE_EVENT_BUFFERINGSTARTED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::BufferingDataStarted;
			} break;
			// NOTE: The media source has stopped buffering data
			case MF_MEDIA_ENGINE_EVENT_BUFFERINGENDED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::BufferingDataEnded;
			} break;
			// NOTE: The IMFMediaEngineEx::FrameStep method completed
			case MF_MEDIA_ENGINE_EVENT_FRAMESTEPCOMPLETED:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackFrameStepCompleted;
			} break;
			// NOTE: The Media Engine's Load algorithm is waiting to start (is created with the MF_MEDIA_ENGINE_WAITFORSTABLE_STATE flag)
			case MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE:
			{
				// ...
			} break;
			// NOTE: The first frame of the media source is ready to render
			case MF_MEDIA_ENGINE_EVENT_FIRSTFRAMEREADY:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::PlaybackFirstFrameReady;
			} break;
			// NOTE: Raised when a new track is added or removed
			case MF_MEDIA_ENGINE_EVENT_TRACKSCHANGE:
			{
				// ...
			} break;
			// NOTE: Raised when there is new information about the Output Protection Manager (OPM)
			case MF_MEDIA_ENGINE_EVENT_OPMINFO:
			{
				// ...
			} break;
			// NOTE: ---
			case MF_MEDIA_ENGINE_EVENT_RESOURCELOST:
			{
				// ...
			} break;
			// NOTE: ---
			case MF_MEDIA_ENGINE_EVENT_DELAYLOADEVENT_CHANGED:
			{
				// ...
			} break;
			// NOTE: Raised when one of the component streams of a media stream fails. This event is only raised if the media stream contains other component streams that did not fail
			case MF_MEDIA_ENGINE_EVENT_STREAMRENDERINGERROR:
			{
				userCallbackParam.Event = MoviePlayerAsyncCallbackEvent::ErrorStreamRendering;
			} break;
			// NOTE: ---
			case MF_MEDIA_ENGINE_EVENT_SUPPORTEDRATES_CHANGED:
			{
				// ...
			} break;
			// NOTE: ---
			case MF_MEDIA_ENGINE_EVENT_AUDIOENDPOINTCHANGE:
			{
				// ...
			} break;
			default:
				assert(false);
				break;
			}

			if (userCallbackParam.Event != MoviePlayerAsyncCallbackEvent::Unknown && userAsyncCallbackFunc)
			{
				const auto userCallbackResult = userAsyncCallbackFunc(userCallbackParam);
				static_cast<void>(userCallbackResult);
			}

			return S_OK;
		}

	private:
		static inline struct GlobalData
		{
			HRESULT CoInitializeResult = {};
			HRESULT MFStartupResult = {};
			bool MediaFoundationInitialized = false;

			bool D3D11DeviceInitialized = false;
			UINT DXGIDeviceManagerID = 0;
			ComPtr<IMFDXGIDeviceManager> DXGIDeviceManager = nullptr;
		} global = {};

		struct AtomicData
		{
			std::atomic<bool> CurrentlyLoadingMetadata = false;
			std::atomic<bool> FrameTextureBufferContentInvalidated = false;

			std::atomic<bool> IsScrubbing = false;
			std::atomic<f32> PreScrubbingPlaybackSpeed = 1.0f;
			std::atomic<f32> PrePausePlaybackSpeed = 1.0f;

			std::atomic<MF_MEDIA_ENGINE_ERR> LastMediaEngineError = MF_MEDIA_ENGINE_ERR_NOERROR;
			std::atomic<HRESULT> LastMediaEngineHResult = S_OK;

			std::atomic<ivec2> VideoResolution = {};
			std::atomic<MoviePlayerStreamAttributes> StreamAttributes = {};
		} atomic = {};

		D3D11& d3d11;
		bool wasInitialized = false;
		ComPtr<MediaEngineNotifyFunctionWrapper> mediaEngineNotifyFunctionWrapper = nullptr;
		ComPtr<IMFMediaEngine> mediaEngine = nullptr;
		ComPtr<IMFMediaEngineEx> mediaEngineEx = nullptr;

		MoviePlayerAsyncCallbackFunc userAsyncCallbackFunc = {};

		std::string videoFilePath;
		ComPtr<ContiniousMemoryBufferMFByteStream> entireFileContentMemoryByteStream = nullptr;

		DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		i32 textureBufferIndex = 0, textureBufferIndexLast = 0;
		std::array<D3D11MovieFrameTextureData, 2> frameTextureBuffer = {};
	};

	std::unique_ptr<IMoviePlayer> MakeD3D11MediaFoundationMediaEngineMoviePlayer()
	{
		HRESULT hr = S_OK;

		auto moviePlayer = std::make_unique<D3D11MediaFoundationMediaEngineMoviePlayer>(GlobalD3D11);
		if (moviePlayer != nullptr)
			hr = moviePlayer->Initialize();

		return moviePlayer;
	}
}
