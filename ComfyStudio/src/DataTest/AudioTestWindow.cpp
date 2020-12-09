#include "AudioTestWindow.h"
#include "Core/Application.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "Input/Input.h"
#include "Time/TimeUtilities.h"
#include "IO/Path.h"
#include <numeric>

namespace Comfy::Studio::DataTest
{
	using namespace Audio;

	AudioTestWindow::AudioTestWindow(Application& parent) : BaseWindow(parent)
	{
		Close();
	}

	AudioTestWindow::~AudioTestWindow()
	{
		if (previewVoiceAdded)
			AudioEngine::GetInstance().RemoveVoice(sourcePreviewVoice);
	}

	const char* AudioTestWindow::GetName() const
	{
		return "Audio Test";
	}

	ImGuiWindowFlags AudioTestWindow::GetFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AudioTestWindow::Gui()
	{
		if (!AudioEngine::InstanceValid())
			return;

		Gui::BeginChild("AudioTestWindowOutterChild", vec2(0.0f, 0.0f), true);
		if (Gui::BeginTabBar("AudioTestWindowTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip))
		{
			AudioEngineGui();
			ActiveVoicesGui();
			LoadedSourcesGui();
			Gui::EndTabBar();
		}
		Gui::EndChild();
	}

	void AudioTestWindow::AudioEngineGui()
	{
		auto& engine = AudioEngine::GetInstance();

		if (Gui::BeginTabItem("Audio Engine"))
		{
			Gui::BeginChild("AudioEngineChild", vec2(0.0f, 0.0f), true);
			{
				GuiPropertyRAII::PropertyValueColumns columns;

				static constexpr auto greenColor = vec4(0.14f, 0.78f, 0.21f, 1.00f);
				static constexpr auto redColor = vec4(0.95f, 0.12f, 0.12f, 1.00f);

				GuiProperty::TreeNode("Audio Engine", ImGuiTreeNodeFlags_DefaultOpen, [&]
				{
					GuiProperty::PropertyLabelValueFunc("Master Volume", [&]
					{
						GuiPropertyRAII::ItemWidth width(-1.0f);

						auto v = engine.GetMasterVolume() * 100.0f;
						if (Gui::SliderFloat("##MasterVolumeSlider", &v, AudioEngine::MinVolume * 100.0f, AudioEngine::MaxVolume * 100.0f, "%.0f%%"))
							engine.SetMasterVolume(v / 100.0f);

						return false;
					});

					GuiProperty::PropertyLabelValueFunc("Device Control", [&]
					{
						if (Gui::Button("Open Start Stream", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
							engine.OpenStartStream();
						if (Gui::Button("Stop Close Stream", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
							engine.StopCloseStream();
						return false;
					});

					const bool isStreamOpen = engine.GetIsStreamOpenRunning();
					GuiProperty::PropertyLabelValueFunc("Device State", [&]
					{
						if (isStreamOpen)
							Gui::TextColored(greenColor, "Open");
						else
							Gui::TextColored(redColor, "Closed");
						return false;
					});

					auto audioBackendIndex = static_cast<int>(engine.GetAudioBackend());
					if (GuiProperty::Combo("Audio Backend", audioBackendIndex, AudioBackendNames))
						engine.SetAudioBackend(static_cast<AudioBackend>(audioBackendIndex));

					auto mixingBehaviorIndex = static_cast<int>(engine.GetChannelMixer().GetMixingBehavior());
					if (GuiProperty::Combo("Channel Mixing", mixingBehaviorIndex, ChannelMixer::MixingBehaviorNames))
						engine.GetChannelMixer().SetMixingBehavior(static_cast<ChannelMixer::MixingBehavior>(mixingBehaviorIndex));

					GuiProperty::TreeNode("Engine Output", ImGuiTreeNodeFlags_DefaultOpen, [&]
					{
						GuiProperty::PropertyLabelValueFunc("Channel Count", [&]
						{
							Gui::Text("%u Channel(s)", engine.GetChannelCount());
							return false;
						});

						GuiProperty::PropertyLabelValueFunc("Sample Rate", [&]
						{
							Gui::Text("%u Hz", engine.GetSampleRate());
							return false;
						});

						GuiProperty::PropertyLabelValueFunc("Callback Frequency", [&]
						{
							Gui::TextColored(isStreamOpen ? greenColor : redColor, "%.3f ms", engine.GetCallbackFrequency().TotalMilliseconds());
							return false;
						});

						GuiProperty::PropertyLabelValueFunc("Buffer Size", [&]
						{
							constexpr u32 slowStep = 8, fastStep = 64;
							GuiPropertyRAII::ItemWidth width(-1.0f);

							Gui::PushItemDisabledAndTextColor();
							u32 currentBufferFrameCount = engine.GetBufferFrameSize();
							Gui::InputScalar("##CurrentBufferSize", ImGuiDataType_U32, &currentBufferFrameCount, &slowStep, &fastStep, "%u Frames (Current)");
							Gui::PopItemDisabledAndTextColor();

							Gui::InputScalar("##NewBufferSize", ImGuiDataType_U32, &newBufferFrameCount, &slowStep, &fastStep, "%u Frames (Request)");
							newBufferFrameCount = std::clamp(newBufferFrameCount, AudioEngine::MinBufferFrameCount, AudioEngine::MaxBufferFrameCount);

							if (Gui::Button("Request Resize", vec2(Gui::CalcItemWidth(), 0.0f)))
								engine.SetBufferFrameSize(newBufferFrameCount);

							return false;
						});

						GuiProperty::PropertyLabelValueFunc("Callback Performance", [&]
						{
							const auto durations = engine.DebugGetCallbackDurations();

							std::array<f32, durations.size()> durationsMS;
							for (size_t i = 0; i < durations.size(); i++)
								durationsMS[i] = static_cast<f32>(durations[i].TotalMilliseconds());

							const f32 totalBufferProcessTimeMS = std::accumulate(durationsMS.begin(), durationsMS.end(), 0.0f);
							const f32 averageProcessTimeMS = (totalBufferProcessTimeMS / static_cast<f32>(durationsMS.size()));

							char overlayTextBuffer[32];
							sprintf_s(overlayTextBuffer, "Average: %.6f ms", averageProcessTimeMS);

							Gui::PlotLines("##CallbackProcessDuration", durationsMS.data(), static_cast<int>(durationsMS.size()), 0, overlayTextBuffer, FLT_MAX, FLT_MAX, vec2(Gui::GetContentRegionAvailWidth(), 32.0f));
							return false;
						});

						GuiProperty::PropertyLabelValueFunc("Render Output", [&]
						{
							Gui::PushStyleColor(ImGuiCol_PlotLines, Gui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
							Gui::PushStyleColor(ImGuiCol_PlotLinesHovered, Gui::GetStyleColorVec4(ImGuiCol_PlotHistogramHovered));

							const auto lastPlayedSamples = engine.DebugGetLastPlayedSamples();
							for (size_t channel = 0; channel < lastPlayedSamples.size(); channel++)
							{
								std::array<f32, lastPlayedSamples[0].size()> normalizedSamples;

								for (size_t sample = 0; sample < normalizedSamples.size(); sample++)
									normalizedSamples[sample] = static_cast<f32>(lastPlayedSamples[channel][sample]) / static_cast<f32>(std::numeric_limits<i16>::max());

								char plotName[32];
								sprintf_s(plotName, "Channel [%zu]", channel);

								GuiPropertyRAII::ID id(static_cast<int>(channel));
								Gui::PlotLines("##ChannelOutputPlot", normalizedSamples.data(), static_cast<int>(normalizedSamples.size()), 0, plotName, -1.0f, 1.0f, vec2(Gui::GetContentRegionAvailWidth(), 32.0f));
							}

							Gui::PopStyleColor(2);
							return false;
						});
					});
				});

				GuiProperty::TreeNode("Audio Engine (Debug)", [&]
				{
					GuiProperty::PropertyLabelValueFunc("Raw Recording", [&]
					{
						if (!engine.DebugGetEnableOutputCapture())
						{
							Gui::PushStyleColor(ImGuiCol_Text, greenColor);
							if (Gui::Button("Start Recording", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
								engine.DebugSetEnableOutputCapture(true);
						}
						else
						{
							Gui::PushStyleColor(ImGuiCol_Text, redColor);
							if (Gui::Button("Stop Recording (Dump)", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
							{
								engine.DebugSetEnableOutputCapture(false);
								const auto filePath = IO::Path::Combine("dev_ram/audio_dump", IO::Path::ChangeExtension("engine_output_" + FormatFileNameDateTimeNow(), ".wav"));
								engine.DebugFlushCaptureToWaveFile(filePath);
							}
						}
						Gui::PopStyleColor();
						return false;
					});

					GuiProperty::PropertyLabelValueFunc("Control Panel", [&]
					{
						if (Gui::Button("Show Control Panel (ASIO)", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
							engine.DebugShowControlPanel();
						return false;
					});
				});
			}
			Gui::EndChild();
			Gui::EndTabItem();
		}
	}

	void AudioTestWindow::ActiveVoicesGui()
	{
		auto& engine = AudioEngine::GetInstance();

		if (Gui::BeginTabItem("Active Voices"))
		{
			static constexpr std::array voicePropertyNames =
			{
				"Name",
				"Position | Duration | Volume | Speed | Voice | Source",
				"Flags",
			};

			Gui::BeginChild("ActiveVoicesChild", vec2(0.0f, 0.0f), true);
			Gui::BeginColumns("AudioVoicesColumns", static_cast<int>(voicePropertyNames.size()), ImGuiColumnsFlags_NoPreserveWidths);

			std::array<Voice, AudioEngine::MaxSimultaneousVoices> allVoices;
			size_t voiceCount = 0;
			engine.DebugGetAllVoices(allVoices.data(), &voiceCount);

			for (size_t i = 0; i < voicePropertyNames.size(); i++)
			{
				Gui::TextUnformatted(Gui::StringViewStart(voicePropertyNames[i]), Gui::StringViewEnd(voicePropertyNames[i]));
				Gui::NextColumn();
			}
			Gui::Separator();

			for (size_t i = 0; i < voiceCount; i++)
			{
				const auto voice = allVoices[i];
				const bool isPlaying = voice.GetIsPlaying();

				if (!isPlaying)
					Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));

				const auto name = voice.GetName();
				Gui::TextUnformatted(Gui::StringViewStart(name), Gui::StringViewEnd(name));
				Gui::NextColumn();

				static_assert(sizeof(HandleBaseType) == 2, "TODO: Update format strings");

				Gui::Text(
					"%s | %s | %.0f%% | %.0f%% | 0x%04X | 0x%04X",
					voice.GetPosition().FormatTime().data(),
					voice.GetDuration().FormatTime().data(),
					voice.GetVolume() * 100.0f,
					voice.GetPlaybackSpeed() * 100.0f,
					static_cast<HandleBaseType>(voice.Handle),
					static_cast<HandleBaseType>(voice.GetSource())
				);
				Gui::NextColumn();

				if (isPlaying) voiceFlagsBuffer += "Playing | ";
				if (voice.GetIsLooping()) voiceFlagsBuffer += "Loop | ";
				if (voice.GetPlayPastEnd()) voiceFlagsBuffer += "PlayPastEnd | ";
				if (voice.GetRemoveOnEnd()) voiceFlagsBuffer += "RemoveOnEnd | ";
				if (voice.GetPauseOnEnd()) voiceFlagsBuffer += "PauseOnEnd | ";

				if (voiceFlagsBuffer.empty())
					Gui::TextUnformatted("None");
				else
					Gui::TextUnformatted(Gui::StringViewStart(voiceFlagsBuffer), Gui::StringViewEnd(voiceFlagsBuffer) - 3);
				Gui::NextColumn();

				voiceFlagsBuffer.clear();

				if (!isPlaying)
					Gui::PopStyleColor();

				Gui::Separator();
			}

			Gui::EndColumns();
			Gui::EndChild();
			Gui::EndTabItem();
		}
	}

	void AudioTestWindow::LoadedSourcesGui()
	{
		auto& engine = AudioEngine::GetInstance();

		if (Gui::BeginTabItem("Loaded Sources"))
		{
			static constexpr std::array sourcePropertyNames =
			{
				"Name",
				"Handle",
				"Base Volume",
			};

			Gui::BeginChild("LoadedSourcesChild", vec2(0.0f, 0.0f), true);
			Gui::BeginColumns("LoadedSourcesColumns", static_cast<int>(sourcePropertyNames.size()), ImGuiColumnsFlags_NoPreserveWidths);

			for (size_t i = 0; i < sourcePropertyNames.size(); i++)
			{
				Gui::TextUnformatted(Gui::StringViewStart(sourcePropertyNames[i]), Gui::StringViewEnd(sourcePropertyNames[i]));
				Gui::NextColumn();
			}
			Gui::Separator();

			// HACK: This is all incredibly inefficient but should be fine for a quick and hacky test window
			for (size_t i = 0; i < engine.DebugGetMaxSourceCount(); i++)
			{
				auto sourceHandle = static_cast<SourceHandle>(i);
				auto sharedSource = engine.GetSharedSource(sourceHandle);

				if (sharedSource != nullptr)
				{
					Gui::PushID(sharedSource.get());
					const bool thisSourceIsPreviewing = (sourcePreviewVoice.GetIsPlaying() && sourcePreviewVoice.GetSource() == sourceHandle);

					if (!thisSourceIsPreviewing) Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));

					if (Gui::Selectable("##SourcePreview", thisSourceIsPreviewing, ImGuiSelectableFlags_SpanAllColumns))
						thisSourceIsPreviewing ? StopSourcePreview() : StartSourcePreview(sourceHandle);
					Gui::SameLine();

					engine.GetSourceName(sourceHandle, sourceNameBuffer);
					Gui::TextUnformatted(Gui::StringViewStart(sourceNameBuffer), Gui::StringViewEnd(sourceNameBuffer));
					Gui::NextColumn();
					sourceNameBuffer.clear();

					static_assert(sizeof(HandleBaseType) == 2, "TODO: Update format strings");
					Gui::Text("0x%04X", static_cast<HandleBaseType>(sourceHandle));
					Gui::NextColumn();

					const auto baseVolume = engine.GetSourceBaseVolume(sourceHandle);
					Gui::Text("%.2f%%", baseVolume * 100.0f);
					Gui::NextColumn();

					if (!thisSourceIsPreviewing) Gui::PopStyleColor();

					Gui::PopID();
				}

				Gui::Separator();
			}

			Gui::EndColumns();
			Gui::EndChild();
			Gui::EndTabItem();
		}
	}

	void AudioTestWindow::StartSourcePreview(SourceHandle source)
	{
		if (source == SourceHandle::Invalid)
			return;

		Audio::AudioEngine::GetInstance().EnsureStreamRunning();

		if (!previewVoiceAdded)
		{
			previewVoiceAdded = true;
			sourcePreviewVoice = AudioEngine::GetInstance().AddVoice(SourceHandle::Invalid, "AudioTestWindow PreviewVoice", false, 1.0f, false);
			sourcePreviewVoice.SetPauseOnEnd(true);
		}

		sourcePreviewVoice.SetSource(source);
		sourcePreviewVoice.SetPosition(TimeSpan::Zero());
		sourcePreviewVoice.SetIsPlaying(true);
	}

	void AudioTestWindow::StopSourcePreview()
	{
		if (previewVoiceAdded)
			sourcePreviewVoice.SetIsPlaying(false);
	}
}
