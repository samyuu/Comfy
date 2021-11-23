#pragma once
#include "Types.h"
#include "Render/Movie/MoviePlayer.h"
#include "Render/Core/Renderer2D/AetRenderer.h"

namespace Comfy::Studio::Editor
{
	// NOTE: To handle video sync / playback for both the Editor and Playtest Mode
	class ChartMoviePlaybackController : NonCopyable
	{
	public:
		ChartMoviePlaybackController() = default;
		~ChartMoviePlaybackController() = default;

	public:
		// NOTE: Both for when the MoviePlayer gets created/deleted and when a file is opened/closed
		void OnMovieChange(Render::IMoviePlayer* currentMoviePlayer);

		void OnResume(TimeSpan playbackTime);
		void OnPause(TimeSpan playbackTime);
		void OnSeek(TimeSpan newPlaybackTime);

		// NOTE: Every frame to handle things like delayed playback start
		void OnUpdateTick(bool isPlaying, TimeSpan playbackTime, TimeSpan currentMovieOffset, f32 currentPlaybackSpeed);

		// NOTE: Only returns a valid texture if the playback time is >= offset and <= duration
		Render::TexSprView GetCurrentTexture(TimeSpan playbackTime);

		// NOTE: Mostly for simplicity sake, reuploads a new texture if the color changed since the last call
		Render::TexSprView GetPlaceholderTexture(vec4 color);

		bool IsMovieAsyncLoading() const;
		bool IsMoviePlayerValidAndReady() const;
		bool IsMoviePlayerValidAndHasVideo() const;

	private:
		Render::IMoviePlayer* moviePlayer = nullptr;

		TimeSpan movieOffset = {}, movieOffsetLastFrame = {};
		f32 playbackSpeed = 1.0f, playbackSpeedLastFrame = 1.0f;

		bool deferMovieStart = false;
		bool deferMovieResyncAfterReload = false;
		bool deferSingleFrameStep = false;

		struct PlaceholderTexAndSprData
		{
			bool Initialized;
			u32 Color;
			Graphics::Tex Tex;
			Graphics::Spr Spr;
		} placeholderTexAndSpr = {};
	};
}
