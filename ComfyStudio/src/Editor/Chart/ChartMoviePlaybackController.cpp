#include "ChartMoviePlaybackController.h"

namespace Comfy::Studio::Editor
{
	void ChartMoviePlaybackController::OnMovieChange(Render::IMoviePlayer* currentMoviePlayer)
	{
		moviePlayer = currentMoviePlayer;
		playbackSpeed = 1.0f;
		deferMovieStart = false;
		movieHasChangedSinceLastUpdateTick = true;
	}

	void ChartMoviePlaybackController::OnResume(TimeSpan playbackTime)
	{
		if (moviePlayer == nullptr || !moviePlayer->GetHasVideoStream())
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
		if (moviePlayer == nullptr || !moviePlayer->GetHasVideoStream())
			return;

		deferMovieStart = false;
		moviePlayer->SetIsPlayingAsync(false);

		if (playbackTime < movieOffset)
			moviePlayer->SetPositionAsync(TimeSpan::Zero());
		else
			moviePlayer->SetPositionAsync(playbackTime + movieOffset);
	}

	void ChartMoviePlaybackController::OnSeek(TimeSpan newPlaybackTime)
	{
		if (moviePlayer == nullptr || !moviePlayer->GetHasVideoStream())
			return;

		moviePlayer->SetPositionAsync(newPlaybackTime + movieOffset);
	}

	void ChartMoviePlaybackController::OnUpdateTick(bool isPlaying, TimeSpan playbackTime, TimeSpan currentMovieOffset, f32 currentPlaybackSpeed)
	{
		movieOffsetLastFrame = movieOffset;
		movieOffset = currentMovieOffset;

		playbackSpeedLastFrame = playbackSpeed;
		playbackSpeed = currentPlaybackSpeed;

		if (moviePlayer == nullptr || !moviePlayer->GetHasVideoStream())
			return;

		if (movieHasChangedSinceLastUpdateTick)
		{
			if (isPlaying && !moviePlayer->GetIsPlaying())
				OnResume(playbackTime);

			movieHasChangedSinceLastUpdateTick = false;
		}

		if (movieOffset != movieOffsetLastFrame)
			moviePlayer->SetPositionAsync(moviePlayer->GetPosition() + (movieOffset - movieOffsetLastFrame));

		if (playbackSpeed != playbackSpeedLastFrame)
			moviePlayer->SetPlaybackSpeedAsync(playbackSpeed);

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
		if (moviePlayer == nullptr || !moviePlayer->GetHasVideoStream())
			return Render::TexSprView { nullptr, nullptr };

		if ((playbackTime < -movieOffset) || ((playbackTime + movieOffset) > moviePlayer->GetDuration()))
			return Render::TexSprView { nullptr, nullptr };

		return moviePlayer->GetCurrentTextureAsTexSprView();
	}
}
