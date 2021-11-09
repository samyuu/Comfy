#include "MovieTestWindow.h"
#include "Core/ComfyStudioApplication.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include "Input/Input.h"
#include "IO/Path.h"

namespace Comfy::Studio::DataTest
{
	MovieTestWindow::MovieTestWindow(ComfyStudioApplication& parent) : BaseWindow(parent)
	{
		Close();
	}

	const char* MovieTestWindow::GetName() const
	{
		return "Movie Test";
	}

	ImGuiWindowFlags MovieTestWindow::GetFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void MovieTestWindow::Gui()
	{
		Gui::BeginChild("MovieTestWindowOutterChild", vec2(0.0f, 0.0f), true);
		{
			movieViews.erase(std::remove_if(movieViews.begin(), movieViews.end(), [](auto& v) { return v.RemoveNextFrame; }), movieViews.end());

			if (Gui::Button("Push Movie View") || isFirstFrame)
			{
				auto& movieView = movieViews.emplace_back();
				movieView.Player = Render::MakeD3D11MediaFoundationMediaEngineMoviePlayer();
				if (movieViews.size() == 1)
				{
					movieView.FilePathBuffer = u8"dev_ram/movie/test_movie.mp4";
					movieView.ReloadNextFrame = true;
				}

				isFirstFrame = false;
			}
			Gui::SameLine();
			{
				const bool noViewsSoFar = movieViews.empty();
				Gui::PushItemDisabledAndTextColorIf(noViewsSoFar);
				if (Gui::Button("Pop Movie View"))
					movieViews.back().RemoveNextFrame = true;
				Gui::PopItemDisabledAndTextColorIf(noViewsSoFar);
			}

			if (movieViews.empty())
				Gui::Separator();

			for (auto& movieView : movieViews)
			{
				Gui::Separator();
				MovieViewGui(movieView);
			}
		}
		Gui::EndChild();
	}

	void MovieTestWindow::MovieViewGui(MovieViewData& movieView)
	{
		Gui::PushID(&movieView);

		{
			if (Gui::Checkbox("Load Fully into Memory", &movieView.LoadFileFullyIntoMemory))
				movieView.ReloadNextFrame = true;

			if (Gui::PathInputTextWithHint("File Path", "movie.mp4", &movieView.FilePathBuffer, ImGuiInputTextFlags_EnterReturnsTrue))
				movieView.ReloadNextFrame = true;
		}

		{
			constexpr vec2 videoDisplaySize = (vec2(1280.0f, 720.0f) * 0.5f);
			const ivec2 videoResolution = movieView.Player->GetResolution();
			const ImTextureID videoTexture = movieView.Player->GetCurrentTexture();

			const auto backgroundButtonCursorScreenPosition = Gui::GetCursorScreenPos();
			Gui::PushItemDisabledAndTextColor();
			Gui::Button((videoTexture == nullptr) ? "(No Frame)" : "##BackgroundButton", videoDisplaySize);
			Gui::PopItemDisabledAndTextColor();

			if (videoTexture != nullptr)
			{
				const auto aspectRatioAdjusted = Gui::FitFixedAspectRatioImage(ImRect(vec2(0.0f, 0.0f), videoDisplaySize), vec2(videoResolution));
				Gui::SetCursorScreenPos(backgroundButtonCursorScreenPosition + aspectRatioAdjusted.Min);
				Gui::Image(videoTexture, aspectRatioAdjusted.GetSize());
			}
		}

		{
			const TimeSpan position = movieView.Player->GetPosition();
			const TimeSpan duration = movieView.Player->GetDuration();

			f64 positionSec = position.TotalSeconds();
			const f64 minSec = 0.0;
			const f64 durationSec = duration.TotalSeconds();

			char timeBuffer[64];
			sprintf_s(timeBuffer, "%s / %s", position.FormatTime().data(), duration.FormatTime().data());

			if (Gui::SliderScalar("##PlaybackPosition", ImGuiDataType_Double, &positionSec, &minSec, &durationSec, timeBuffer, ImGuiSliderFlags_None))
				movieView.Player->SetPositionAsync(TimeSpan::FromSeconds(positionSec));
		}

		{
			constexpr vec2 controlButtonSize = vec2(72.0f, 0.0f);
			{
				Gui::PushButtonRepeat(true);
				if (Gui::ButtonEx("-1 Frame", controlButtonSize))
					movieView.Player->FrameStepAsync(false);
				Gui::PopButtonRepeat();
			}
			Gui::SameLine();
			{
				const bool isPlaying = movieView.Player->GetIsPlaying();
				if (Gui::ButtonEx(isPlaying ? "Pause" : "Play", controlButtonSize))
					movieView.Player->SetIsPlayingAsync(!isPlaying);
			}
			Gui::SameLine();
			{
				Gui::PushButtonRepeat(true);
				if (Gui::ButtonEx("Frame +1", controlButtonSize))
					movieView.Player->FrameStepAsync(true);
				Gui::PopButtonRepeat();
			}
			Gui::SameLine();
			{
				Gui::SetNextItemWidth(controlButtonSize.x * 2.0f);
				f32 playbackSpeed = movieView.Player->GetPlaybackSpeed();
				if (Gui::SliderFloat("##PlaybackSpeed", &playbackSpeed, 0.0f, 2.0f, "%.2fx Speed"))
					movieView.Player->SetPlaybackSpeedAsync(playbackSpeed);
			}
			Gui::SameLine();
			{
				bool isLooping = movieView.Player->GetIsLooping();
				if (Gui::Checkbox("Loop", &isLooping))
					movieView.Player->SetIsLoopingAsync(isLooping);
			}
		}
		Gui::Separator();
		{
			Render::MoviePlayerStreamAttributes streamAttributes = {};
			if (movieView.Player->TryGetStreamAttributes(streamAttributes))
			{
				Gui::Text("MF_PD_LAST_MODIFIED_TIME: %llu", streamAttributes.LastModifiedFileTime);
				Gui::Text("MF_PD_TOTAL_FILE_SIZE: %llu", streamAttributes.TotalFileSize);
				Gui::Separator();
				{
					Gui::PushItemDisabledAndTextColorIf(!streamAttributes.HasAudioStream);
					Gui::Text("MF_MT_AUDIO_AVG_BYTES_PER_SECOND: %lu", streamAttributes.Audio.AverageBytesPerSecond);
					Gui::Text("MF_MT_AUDIO_BITS_PER_SAMPLE: %lu", streamAttributes.Audio.BitsPerSample);
					Gui::Text("MF_MT_AUDIO_BLOCK_ALIGNMENT: %lu", streamAttributes.Audio.BlockAlignment);
					Gui::Text("MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND: %g", streamAttributes.Audio.FloatSamplesPerSecond);
					Gui::Text("MF_MT_AUDIO_NUM_CHANNELS: %lu", streamAttributes.Audio.ChannelCount);
					Gui::Text("MF_MT_AUDIO_SAMPLES_PER_BLOCK: %lu", streamAttributes.Audio.SamplesPerBlock);
					Gui::Text("MF_MT_AUDIO_SAMPLES_PER_SECOND: %lu", streamAttributes.Audio.SamplesPerSecond);
					Gui::PopItemDisabledAndTextColorIf(!streamAttributes.HasAudioStream);
				}
				Gui::Separator();
				{
					Gui::PushItemDisabledAndTextColorIf(!streamAttributes.HasVideoStream);
					Gui::Text("MF_MT_AVG_BITRATE: %lu", streamAttributes.Video.AverageBitRate);
					Gui::Text("MF_MT_FRAME_RATE: %lu/%lu", streamAttributes.Video.FrameRateNumerator, streamAttributes.Video.FrameRateDenominator);
					Gui::Text("MF_MT_FRAME_SIZE: %lux%lu", streamAttributes.Video.FrameSizeWidth, streamAttributes.Video.FrameSizeHeight);
					Gui::Text("MF_MT_MAX_KEYFRAME_SPACING: %lu", streamAttributes.Video.MaxKeyFrameSpacing);
					Gui::PopItemDisabledAndTextColorIf(!streamAttributes.HasVideoStream);
				}
				Gui::Separator();
			}
		}

		if (movieView.ReloadNextFrame)
		{
			if (movieView.LoadFileFullyIntoMemory)
			{
				auto[fileContent, fileSize] = IO::File::ReadAllBytes(movieView.FilePathBuffer);
				movieView.Player->OpenFileBytesAsync(movieView.FilePathBuffer, std::move(fileContent), fileSize);
			}
			else
			{
				movieView.Player->OpenFileAsync(movieView.FilePathBuffer);
			}
			movieView.Player->SetIsPlayingAsync(true);
			movieView.ReloadNextFrame = false;
		}

		Gui::PopID();
	}
}
