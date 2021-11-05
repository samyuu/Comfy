#pragma once
#include "Types.h"
#include "Time/TimeSpan.h"
#include "Misc/UTF8.h"
#include "ImGui/Gui.h"
#include <functional>

namespace Comfy::Render
{
	struct TexSprView;

	struct MoviePlayerStreamAttributes
	{
		// MF_PD_DURATION
		u64 PresentationDurationMFTime;
		// MF_PD_LAST_MODIFIED_TIME
		u64 LastModifiedFileTime;
		// MF_PD_TOTAL_FILE_SIZE
		u64 TotalFileSize;

		bool HasAudioStream;
		bool HasVideoStream;

		struct AudioStreamData
		{
			// MF_MT_AUDIO_AVG_BYTES_PER_SECOND
			u32 AverageBytesPerSecond;
			// MF_MT_AUDIO_BITS_PER_SAMPLE
			u32 BitsPerSample;
			// MF_MT_AUDIO_BLOCK_ALIGNMENT
			u32 BlockAlignment;
			// MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND
			f64 FloatSamplesPerSecond;
			// MF_MT_AUDIO_NUM_CHANNELS 
			u32 ChannelCount;
			// MF_MT_AUDIO_SAMPLES_PER_BLOCK
			u32 SamplesPerBlock;
			// MF_MT_AUDIO_SAMPLES_PER_SECOND
			u32 SamplesPerSecond;
		} Audio;

		struct VideoStreamData
		{
			// MF_MT_AVG_BITRATE
			u32 AverageBitRate;
			// MF_MT_FRAME_RATE
			u32 FrameRateNumerator;
			u32 FrameRateDenominator;
			// MF_MT_FRAME_SIZE 
			u32 FrameSizeWidth;
			u32 FrameSizeHeight;
			// MF_MT_MAX_KEYFRAME_SPACING 
			u32 MaxKeyFrameSpacing;
		} Video;
	};

	enum class MoviePlayerAsyncCallbackEvent : u32
	{
		Unknown,

		LoadStarted,
		LoadInProgress,
		LoadSuspended,
		LoadAborted,
		LoadStalled,

		LoadedMetadata,
		LoadedData,

		PlaybackStarted,
		PlaybackSwitchedToPlay,
		PlaybackSwitchedToPause,
		PlaybackPausedWaitingForNextFrame,
		PlaybackCanPlay,
		PlaybackCanPlayThroughToEnd,
		PlaybackSeekStart,
		PlaybackSeekEnd,
		PlaybackFrameStepCompleted,
		PlaybackPositionChanged,
		PlaybackReachedEnd,
		PlaybackSpeedChanged,
		PlaybackFirstFrameReady,

		BufferingDataStarted,
		BufferingDataEnded,
		EventQueueCleared,

		ErrorStreamRendering,
		Error,

		Count
	};

	struct MoviePlayerAsyncCallbackParam
	{
		class IMoviePlayer* MoviePlayer;
		MoviePlayerAsyncCallbackEvent Event;
		union EventUnionData
		{
			struct
			{
				TimeSpan NewTime;
			} PlaybackPositionChanged;
			struct
			{
				f32 NewSpeed;
			} PlaybackSpeedChanged;
		} EventData;
	};

	struct MoviePlayerAsyncCallbackResult { u8 Reserved; };

	using MoviePlayerAsyncCallbackFunc = std::function<MoviePlayerAsyncCallbackResult(MoviePlayerAsyncCallbackParam param)>;

	class IMoviePlayer : NonCopyable
	{
	public:
		IMoviePlayer() = default;
		virtual ~IMoviePlayer() = default;

	public:
		virtual bool OpenFileAsync(std::string_view filePath) = 0;
		virtual bool OpenFileBytesAsync(std::string_view fileName, std::unique_ptr<u8[]> fileBytes, size_t fileSize) = 0;
		virtual bool CloseFileAsync() = 0;

		virtual bool GetIsLoadingFileAsync() const = 0;
		virtual std::string GetFilePath() const = 0;
		virtual void WaitUntilFileOpenCompletedSync(TimeSpan timeoutDuration) = 0;

		virtual bool GetIsPlaying() const = 0;
		virtual bool SetIsPlayingAsync(bool value) = 0;

		virtual bool GetIsSeeking() const = 0;

		virtual f32 GetPlaybackSpeed() const = 0;
		virtual bool SetPlaybackSpeedAsync(f32 value) = 0;

		virtual bool GetIsScrubbing() const = 0;
		virtual bool SetIsScrubbingAsync(bool value) = 0;

		virtual bool GetIsPlaybackSpeedSupported(f32 value) = 0;

		virtual TimeSpan GetPosition() const = 0;
		virtual bool SetPositionAsync(TimeSpan value, bool accurate = true) = 0;

		virtual bool FrameStepAsync(bool forward) = 0;

		virtual TimeSpan GetDuration() const = 0;
		virtual ivec2 GetResolution() const = 0;

		virtual bool GetIsLooping() const = 0;
		virtual bool SetIsLoopingAsync(bool value) = 0;

		virtual bool GetHasVideoStream() const = 0;
		virtual bool GetHasAudioStream() const = 0;

		virtual bool RegisterAsyncCallback(MoviePlayerAsyncCallbackFunc callbackFunc) = 0;
		virtual bool TryGetStreamAttributes(MoviePlayerStreamAttributes& outAttributes) const = 0;

		virtual ComfyTextureID GetCurrentTexture() = 0;
		virtual TexSprView GetCurrentTextureAsTexSprView() = 0;
	};

	std::unique_ptr<IMoviePlayer> MakeD3D11MediaFoundationMediaEngineMoviePlayer();
}
