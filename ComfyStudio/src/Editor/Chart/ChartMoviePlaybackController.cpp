#include "ChartMoviePlaybackController.h"

namespace Comfy::Studio::Editor
{
	void ChartMoviePlaybackController::OnMovieChange(Render::IMoviePlayer* currentMoviePlayer)
	{
		moviePlayer = currentMoviePlayer;
		playbackSpeed = 1.0f;
		deferMovieStart = false;
		deferMovieResyncAfterReload = true;
		deferSingleFrameStep = true;
	}

	void ChartMoviePlaybackController::OnResume(TimeSpan playbackTime)
	{
		deferMovieStart = false;
		if (!IsMoviePlayerValidAndHasVideo())
			return;

		if (playbackTime < -movieOffset)
		{
			deferMovieStart = true;
		}
		else
		{
			moviePlayer->SetIsPlayingAsync(true);
			if (moviePlayer->GetPlaybackSpeed() != playbackSpeed)
				moviePlayer->SetPlaybackSpeedAsync(playbackSpeed);
		}
	}

	void ChartMoviePlaybackController::OnPause(TimeSpan playbackTime)
	{
		deferMovieStart = false;
		if (!IsMoviePlayerValidAndHasVideo())
			return;

		deferMovieResyncAfterReload = false;
		moviePlayer->SetIsPlayingAsync(false);

		if (playbackTime < movieOffset)
			moviePlayer->SetPositionAsync(TimeSpan::Zero());
		else
			moviePlayer->SetPositionAsync(playbackTime + movieOffset);
	}

	void ChartMoviePlaybackController::OnSeek(TimeSpan newPlaybackTime)
	{
		if (!IsMoviePlayerValidAndHasVideo())
			return;

		moviePlayer->SetPositionAsync(newPlaybackTime + movieOffset);
	}

	void ChartMoviePlaybackController::OnUpdateTick(bool isPlaying, TimeSpan playbackTime, TimeSpan currentMovieOffset, f32 currentPlaybackSpeed)
	{
		movieOffsetLastFrame = movieOffset;
		movieOffset = currentMovieOffset;

		playbackSpeedLastFrame = playbackSpeed;
		playbackSpeed = currentPlaybackSpeed;

		if (!IsMoviePlayerValidAndReady())
			return;

		// HACK: Force a single frame to be rendered using a FrameStep so that subsequent seeks correctly update the underlying frame.
		//		 This is only required if the MoviePlayer hasn't already entered the playing state at least once before.
		//		 Also has the nice side effect of the first unpause hopefully being more responsive
		if (deferSingleFrameStep && !isPlaying)
			moviePlayer->FrameStepAsync(true);
		deferSingleFrameStep = false;

		// NOTE: Important to check for GetHasEnoughData() otherwise the video position gets reset when the first frame is decoded
		if (deferMovieResyncAfterReload && moviePlayer->GetHasEnoughData())
		{
			// BUG: If the IAudioBackend fails to initialize (mostly due to exclusive mode) then the movie keeps playing despite the audio being "stuck"
			OnSeek(playbackTime);
			if (isPlaying && !moviePlayer->GetIsPlaying())
				OnResume(playbackTime);
			deferMovieResyncAfterReload = false;
		}
		else
		{
			// NOTE: Dynamically update the video while the video offset is changed by the user, both while playing and paused
			if (movieOffset != movieOffsetLastFrame)
			{
				if (playbackTime < -movieOffset)
				{
					moviePlayer->SetPositionAsync(TimeSpan::Zero());
					if (isPlaying)
						deferMovieStart = true;
				}
				else
				{
					moviePlayer->SetPositionAsync(playbackTime + movieOffset);
				}
			}

			// NOTE: Dynamically update the video playback speed while changed by the user
			if (playbackSpeed != playbackSpeedLastFrame)
				moviePlayer->SetPlaybackSpeedAsync(playbackSpeed);
		}

		if (isPlaying && deferMovieStart && (playbackTime >= -movieOffset))
		{
			moviePlayer->SetIsPlayingAsync(true);
			if (moviePlayer->GetPlaybackSpeed() != playbackSpeed)
				moviePlayer->SetPlaybackSpeedAsync(playbackSpeed);
			deferMovieStart = false;
		}

		// NOTE: Handle the case of having played past the end, paused and rewound
		if (!isPlaying && (playbackTime + movieOffset) >= moviePlayer->GetDuration() && moviePlayer->GetIsPlaying())
			moviePlayer->SetIsPlayingAsync(false);
	}

	Render::TexSprView ChartMoviePlaybackController::GetCurrentTexture(TimeSpan playbackTime)
	{
		if (!IsMoviePlayerValidAndReady())
			return Render::TexSprView { nullptr, nullptr };

		if ((playbackTime < -movieOffset) || ((playbackTime + movieOffset) > moviePlayer->GetDuration()))
			return Render::TexSprView { nullptr, nullptr };

		return moviePlayer->GetCurrentTextureAsTexSprView();
	}

	Render::TexSprView ChartMoviePlaybackController::GetPlaceholderTexture(vec4 color)
	{
		auto& texAndSpr = placeholderTexAndSpr;

		const u32 newColorU32 = Gui::ColorConvertFloat4ToU32(color);
		if (!texAndSpr.Initialized || texAndSpr.Color != newColorU32)
		{
			texAndSpr.Color = newColorU32;

			auto& baseMip = texAndSpr.Tex.MipMapsArray.empty() ? texAndSpr.Tex.MipMapsArray.emplace_back().emplace_back() : texAndSpr.Tex.MipMapsArray[0][0];
			if (!texAndSpr.Initialized)
			{
				texAndSpr.Tex.Name = "MoviePlaceholderTexture";
				texAndSpr.Tex.GPU_Texture2D.DynamicResource = true;

				baseMip.Size = { 2, 2 };
				baseMip.Format = Graphics::TextureFormat::RGBA8;
				baseMip.DataSize = (baseMip.Size.x * baseMip.Size.y) * sizeof(u32);
				baseMip.Data = std::make_unique<u8[]>(baseMip.DataSize);

				texAndSpr.Spr.TexelRegion = { 0.5f, 0.5f, 0.5f, 0.5f };
				texAndSpr.Spr.PixelRegion = { 0.5f, 0.5f, 1.0f, 1.0f };
				texAndSpr.Spr.Extra.ScreenMode = Graphics::ScreenMode::HDTV1080;
				texAndSpr.Initialized = true;
			}

			for (i32 i = 0; i < (baseMip.Size.x * baseMip.Size.y); i++)
				reinterpret_cast<u32*>(baseMip.Data.get())[i] = newColorU32;

			texAndSpr.Tex.GPU_Texture2D.RequestReupload = true;
		}

		return { &texAndSpr.Tex, &texAndSpr.Spr };
	}

	bool ChartMoviePlaybackController::IsMovieAsyncLoading() const
	{
		return (moviePlayer != nullptr) && moviePlayer->GetIsLoadingFileAsync();
	}

	bool ChartMoviePlaybackController::IsMoviePlayerValidAndReady() const
	{
		return (moviePlayer != nullptr && moviePlayer->GetHasEnoughData());
	}

	bool ChartMoviePlaybackController::IsMoviePlayerValidAndHasVideo() const
	{
		return (moviePlayer != nullptr && moviePlayer->GetHasVideoStream());
	}
}
