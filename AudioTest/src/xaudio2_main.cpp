#include <stdio.h>
#include <xaudio2.h>
#include <assert.h>

#define assert_failed(x) assert(!FAILED(x));

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

//constexpr const char* FileName = "rom/nop_cracking_open_a_cold_one.wav";
constexpr const char* FileName = "rom/01_button1.wav";

IXAudio2* pXAudio2 = nullptr;
IXAudio2MasteringVoice* pMasterVoice = nullptr;
IXAudio2SourceVoice* pSourceVoice = nullptr;

BYTE* pDataBuffer;
WAVEFORMATEXTENSIBLE wfx = { 0 };
XAUDIO2_BUFFER buffer = { 0 };

void InitializeAudioEngine()
{
	HRESULT result = NULL;

	// create instance of XAudio2 engine
	result = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert_failed(result);

	result = pXAudio2->CreateMasteringVoice(&pMasterVoice);
	assert_failed(result);
}

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;

}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

HRESULT LoadAudioFile()
{
	HANDLE hFile = CreateFile(
		FileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		return HRESULT_FROM_WIN32(GetLastError());

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
		return S_FALSE;

	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
	buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
}

void CreateSourceVoice()
{
	HRESULT result = NULL;

	result = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx);
	assert_failed(result);
}

void SubmitSourceBuffer()
{
	HRESULT result = NULL;

	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	assert_failed(result);
}

int xinput2_main()
{
	printf("main(): Starting...\n");
	
	CoInitialize(nullptr);
	InitializeAudioEngine();

	LoadAudioFile();
	CreateSourceVoice();
	//SubmitSourceBuffer();

	pSourceVoice->Start(0);

	bool playing = false;
	while (true)
	{
		if (GetAsyncKeyState('K') && !playing)
		{
			SubmitSourceBuffer();
			pSourceVoice->Start(0);
			playing = true;
		}

		if (GetAsyncKeyState('I'))
		{
			pSourceVoice->Stop(0);
			playing = false;
		}
	}

	printf("main(): All Done!\n");
	getchar();
}