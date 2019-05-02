/*
 * wave_play_wasapi_shared.cpp
 *
 * Created by fukuroda (https://github.com/fukuroder)
 */

 // windows API
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

// libsndfile -> http://www.mega-nerd.com/libsndfile/
#include "sndfile.h"
#pragma comment(lib, "libsndfile-1.lib")

// STL
#include <iostream>

int wasapi_main()
{
	SNDFILE* snd_file = nullptr;
	IMMDevice *pDevice = nullptr;
	IMMDeviceEnumerator *pDeviceEnumerator = nullptr;
	IAudioClient *pAudioClient = nullptr;
	IAudioRenderClient *pAudioRenderClient = nullptr;
	HANDLE hEvent = nullptr;
	try
	{
		// open sound file (wav/ogg/flac...)
		SF_INFO sf_info = {};
		//snd_file = sf_open("test.flac", SFM_READ, &sf_info);
		snd_file = sf_open("rom/selector_vox_lp.ogg", SFM_READ, &sf_info);

		if (snd_file == nullptr) throw std::runtime_error("sf_open error");

		//if (sf_info.frames == 0 ||
		//	sf_info.channels != 2 ||
		//	sf_info.samplerate != 44100) throw std::runtime_error("44.1kHz/stereo only");

		// COM result
		HRESULT hr = S_OK;

		hr = CoInitialize(nullptr);
		if (FAILED(hr)) throw std::runtime_error("CoInitialize error");

		hr = CoCreateInstance(
			__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator),
			(void**)&pDeviceEnumerator);
		if (FAILED(hr)) throw std::runtime_error("CoCreateInstance error");

		hr = pDeviceEnumerator->GetDefaultAudioEndpoint(
			eRender,
			eConsole,
			&pDevice);
		if (FAILED(hr)) throw std::runtime_error("IMMDeviceEnumerator.GetDefaultAudioEndpoint error");
		std::cout << "IMMDeviceEnumerator.GetDefaultAudioEndpoint()->OK" << std::endl;

		IAudioClient *pAudioClient = nullptr;
		hr = pDevice->Activate(
			__uuidof(IAudioClient),
			CLSCTX_ALL,
			nullptr,
			(void**)&pAudioClient);
		if (FAILED(hr)) throw std::runtime_error("IMMDevice.Activate error");
		std::cout << "IMMDevice.Activate()->OK" << std::endl;

		REFERENCE_TIME DefaultDevicePeriod = 0;
		REFERENCE_TIME MinimumDevicePeriod = 0;
		hr = pAudioClient->GetDevicePeriod(&DefaultDevicePeriod, &MinimumDevicePeriod);
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetDevicePeriod error");
		std::cout << "default device period=" << DefaultDevicePeriod << "[nano seconds]" << std::endl;
		std::cout << "minimum device period=" << MinimumDevicePeriod << "[nano seconds]" << std::endl;

		if (!true)
		{
			//WAVEFORMATEX wave_format = {};
			//wave_format.wFormatTag = WAVE_FORMAT_PCM;
			//wave_format.nChannels = 2;
			//wave_format.nSamplesPerSec = 44100;
			//wave_format.nAvgBytesPerSec = 44100 * 2 * 16 / 8;
			//wave_format.nBlockAlign = 2 * 16 / 8;
			//wave_format.wBitsPerSample = 16;
			WAVEFORMATEX wave_format = {};
			wave_format.wFormatTag = WAVE_FORMAT_PCM;
			wave_format.nChannels = 2;
			wave_format.nSamplesPerSec = 44100;
			wave_format.nAvgBytesPerSec = 44100 * 2 * 16 / 8;
			wave_format.nBlockAlign = 2 * 16 / 8;
			wave_format.wBitsPerSample = 16;

			hr = pAudioClient->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				MinimumDevicePeriod,
				0,
				&wave_format,
				nullptr);

			if (FAILED(hr)) throw std::runtime_error("IAudioClient.Initialize error");
			std::cout << "IAudioClient.Initialize()->OK" << std::endl;
		}
		else
		{
			//WAVEFORMATEX* wave_format;
			//hr = pAudioClient->GetMixFormat(&wave_format);
			////wave_format->wFormatTag = WAVE_FORMAT_PCM;
			////wave_format->nSamplesPerSec = 44100;
			////wave_format->nAvgBytesPerSec = 44100 * 2 * 16 / 8;

			//hr = pAudioClient->Initialize(
			//	AUDCLNT_SHAREMODE_SHARED,
			//	AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			//	MinimumDevicePeriod,
			//	0,
			//	wave_format,
			//	nullptr);

			//if (FAILED(hr)) throw std::runtime_error("IAudioClient.Initialize error");
			//std::cout << "IAudioClient.Initialize()->OK" << std::endl;


			WAVEFORMATEX *pwfx = NULL;

			// Get the device format.
			(hr = pAudioClient->GetMixFormat(&pwfx));

			// Open the stream and associate it with an audio session.
			hr = pAudioClient->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				10000000,
				10000000,
				pwfx,
				NULL);
		}

		// event
		hEvent = CreateEvent(nullptr, false, false, nullptr);
		if (FAILED(hr)) throw std::runtime_error("CreateEvent error");

		hr = pAudioClient->SetEventHandle(hEvent);
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.SetEventHandle error");

		UINT32 NumBufferFrames = 0;
		hr = pAudioClient->GetBufferSize(&NumBufferFrames);
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetBufferSize error");
		std::cout << "buffer frame size=" << NumBufferFrames << "[frames]" << std::endl;

		hr = pAudioClient->GetService(
			__uuidof(IAudioRenderClient),
			(void**)&pAudioRenderClient);
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetService error");

		BYTE *pData = nullptr;
		hr = pAudioRenderClient->GetBuffer(NumBufferFrames, &pData);
		if (FAILED(hr)) throw std::runtime_error("IAudioRenderClient.GetBuffer error");

		sf_count_t read_count = sf_readf_short(snd_file, (short*)pData, NumBufferFrames);

		hr = pAudioRenderClient->ReleaseBuffer((UINT32)read_count, 0);
		if (FAILED(hr)) throw std::runtime_error("IAudioRenderClient.ReleaseBuffer error");

		hr = pAudioClient->Start();
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.Start error");
		std::cout << "IAudioClient.Start()->OK" << std::endl;

		bool playing = (read_count == NumBufferFrames);
		printf("playing...\n");
		while (playing)
		{
			WaitForSingleObject(hEvent, INFINITE);

			UINT32 NumPaddingFrames = 0;
			hr = pAudioClient->GetCurrentPadding(&NumPaddingFrames);
			if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetCurrentPadding error");

			UINT32 numAvailableFrames = NumBufferFrames - NumPaddingFrames;
			if (numAvailableFrames == 0) continue;

			hr = pAudioRenderClient->GetBuffer(numAvailableFrames, &pData);
			if (FAILED(hr)) throw std::runtime_error("IAudioRenderClient.GetBuffer error");

			read_count = sf_readf_short(snd_file, (short*)pData, numAvailableFrames);

			hr = pAudioRenderClient->ReleaseBuffer((UINT32)read_count, 0);
			if (FAILED(hr)) throw std::runtime_error("IAudioRenderClient.ReleaseBuffer error");

			//printf("read_count: %d\n", read_count);
			playing = (read_count == numAvailableFrames);
		}

		printf("finished playing...\n");
		do
		{
			// wait for buffer to be empty
			WaitForSingleObject(hEvent, INFINITE);

			UINT32 NumPaddingFrames = 0;
			hr = pAudioClient->GetCurrentPadding(&NumPaddingFrames);
			if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetCurrentPadding error");

			if (NumPaddingFrames == 0)
			{
				std::cout << "current buffer padding=0[frames]" << std::endl;
				break;
			}
		} while (true);

		hr = pAudioClient->Stop();
		if (FAILED(hr)) throw std::runtime_error("IAudioClient.Stop error");
		std::cout << "IAudioClient.Stop()->OK" << std::endl;

	}
	catch (std::exception& ex)
	{
		std::cout << "error:" << ex.what() << std::endl;

	}

	CloseHandle(hEvent);
	if (pDeviceEnumerator) pDeviceEnumerator->Release();
	if (pDevice) pDevice->Release();
	if (pAudioClient) pAudioClient->Release();
	if (pAudioRenderClient) pAudioRenderClient->Release();
	CoUninitialize();
	sf_close(snd_file);

	return 0;
}