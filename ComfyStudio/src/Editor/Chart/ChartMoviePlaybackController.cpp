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
		if (!IsMoviePlayerValidAndReady())
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
		if (!IsMoviePlayerValidAndReady())
			return;

		deferMovieStart = false;
		deferMovieResyncAfterReload = false;
		moviePlayer->SetIsPlayingAsync(false);

		if (playbackTime < movieOffset)
			moviePlayer->SetPositionAsync(TimeSpan::Zero());
		else
			moviePlayer->SetPositionAsync(playbackTime + movieOffset);
	}

	void ChartMoviePlaybackController::OnSeek(TimeSpan newPlaybackTime)
	{
		if (!IsMoviePlayerValidAndReady())
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
	}

	Render::TexSprView ChartMoviePlaybackController::GetCurrentTexture(TimeSpan playbackTime)
	{
		if (!IsMoviePlayerValidAndReady())
			return Render::TexSprView { nullptr, nullptr };

		if ((playbackTime < -movieOffset) || ((playbackTime + movieOffset) > moviePlayer->GetDuration()))
			return Render::TexSprView { nullptr, nullptr };

		return moviePlayer->GetCurrentTextureAsTexSprView();
	}

	bool ChartMoviePlaybackController::IsMoviePlayerValidAndReady() const
	{
#if 0 // NOTE: Checking either one *seems* to be working correctly so not sure which one makes more sense here...
		return (moviePlayer != nullptr && moviePlayer->GetHasVideoStream());
#else
		return (moviePlayer != nullptr && moviePlayer->GetHasEnoughData());
#endif
	}
}
