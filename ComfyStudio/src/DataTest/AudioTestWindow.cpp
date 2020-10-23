#include "AudioTestWindow.h"
#include "Core/Application.h"
#include "Input/Input.h"
#include "Time/TimeUtilities.h"
#include "IO/Path.h"
#include <numeric>

namespace Comfy::Studio::DataTest
{
	AudioTestWindow::AudioTestWindow(Application& parent) : BaseWindow(parent)
	{
		Close();
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
		auto& engine = Audio::AudioEngine::GetInstance();

		auto checkStartStream = [&]()
		{
			static bool checkStartStream = true;
			if (checkStartStream)
			{
				engine.EnsureStreamRunning();
				checkStartStream = false;
			}
		};

		// TODO: Rework all of this
		Gui::TextUnformatted("Audio Test:");
		Gui::Separator();

		f32 masterVolume = engine.GetMasterVolume();
		if (Gui::SliderFloat("Master Volume", &masterVolume, Audio::AudioEngine::MinVolume, Audio::AudioEngine::MaxVolume))
			engine.SetMasterVolume(masterVolume);
		Gui::Separator();

		if (Gui::CollapsingHeader("Device List"))
		{
			Gui::Text("Device(s): %d", static_cast<int>(deviceInfoList.size()));

			f32 halfWindowWidth = Gui::GetWindowWidth() / 2.0f;

			Gui::SameLine(halfWindowWidth);
			if (Gui::Button("Refresh Device List", vec2(halfWindowWidth, 0.0f)))
				RefreshDeviceInfoList();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(2.0f, 2.0f));
			Gui::Columns(2);

			for (size_t i = 0; i < deviceInfoList.size(); i++)
				ShowDeviceInfoProperties(deviceInfoList.at(i), static_cast<int>(i));

			Gui::Columns(1);
			Gui::PopStyleVar();
		}
		Gui::Separator();

		const auto onColor = ImVec4(0.14f, 0.78f, 0.21f, 1.00f);
		const auto offColor = ImVec4(0.95f, 0.12f, 0.12f, 1.00f);

		if (Gui::CollapsingHeader("Stream Control", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto buttonSize = vec2(Gui::GetWindowWidth() / 2.0f, 0.0);

			Gui::PushItemWidth(buttonSize.x);
			{
				if (Gui::Button("OpenStartStream()", buttonSize))
					engine.OpenStartStream();
				Gui::SameLine();
				if (Gui::Button("StopCloseStream()", buttonSize))
					engine.StopCloseStream();

				bool isStreamOpen = engine.GetIsStreamOpenRunning();
				Gui::Text("%-24s:", "GetIsStreamOpenRunning()");
				Gui::SameLine();
				Gui::TextColored(isStreamOpen ? onColor : offColor, isStreamOpen ? "Open" : "Closed");
			}
			Gui::PopItemWidth();

			Gui::Text("%-25s: %u", "GetChannelCount()", engine.GetChannelCount());
			Gui::Text("%-28s: %u Hz", "GetSampleRate()", engine.GetSampleRate());
			// Gui::Text("%-28s: %.3f sec", "GetStreamTime()", engine.GetStreamTime().TotalSeconds());
			Gui::Text("%-23s: %.3f ms", "GetCallbackFrequency()", engine.GetCallbackFrequency().TotalMilliseconds());

			const auto durations = engine.DebugGetCallbackDurations();

			std::array<f32, durations.size()> durationsMS;
			for (size_t i = 0; i < durations.size(); i++)
				durationsMS[i] = static_cast<f32>(durations[i].TotalMilliseconds());

			const f32 totalBufferProcessTimeMS = std::accumulate(durationsMS.begin(), durationsMS.end(), 0.0f);
			const f32 averageProcessTimeMS = (totalBufferProcessTimeMS / static_cast<f32>(durationsMS.size()));

			char overlayTextBuffer[32];
			sprintf_s(overlayTextBuffer, "Average: %.6f ms", averageProcessTimeMS);

			Gui::PlotLines("Callback Process Duration", durationsMS.data(), static_cast<int>(durationsMS.size()), 0, overlayTextBuffer, FLT_MAX, FLT_MAX, vec2(0.0f, 32.0f));

			Gui::Separator();

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
					sprintf_s(plotName, "Channel Output [%zu]", channel);
					Gui::PlotLines(plotName, normalizedSamples.data(), static_cast<int>(normalizedSamples.size()), 0, nullptr, -1.0f, 1.0f, vec2(0.0f, 32.0f));
				}

				Gui::PopStyleColor(2);
			}

			Gui::Separator();

			Gui::Text("%-21s: %d", "GetBufferFrameSize()", engine.GetBufferFrameSize());

			constexpr u32 slowStep = 8, fastStep = 64;
			Gui::InputScalar("Buffer Size", ImGuiDataType_U32, &newBufferFrameCount, &slowStep, &fastStep, "%u");

			newBufferFrameCount = std::clamp(newBufferFrameCount, Audio::AudioEngine::MinBufferFrameCount, Audio::AudioEngine::MaxBufferFrameCount);

			if (Gui::Button("SetBufferFrameSize()", vec2(Gui::CalcItemWidth(), 0.0f)))
				engine.SetBufferFrameSize(newBufferFrameCount);

			if (!engine.DebugGetEnableOutputCapture())
			{
				Gui::PushStyleColor(ImGuiCol_Text, onColor);
				if (Gui::Button("Start Recording", vec2(Gui::CalcItemWidth(), 0.0f)))
					engine.DebugSetEnableOutputCapture(true);
			}
			else
			{
				Gui::PushStyleColor(ImGuiCol_Text, offColor);
				if (Gui::Button("Stop Recording (Dump)", vec2(Gui::CalcItemWidth(), 0.0f)))
				{
					engine.DebugSetEnableOutputCapture(false);
					const auto filePath = IO::Path::Combine("dev_ram/audio_dump", IO::Path::ChangeExtension("engine_output_" + FormatFileNameDateTimeNow(), ".wav"));
					engine.DebugFlushCaptureToWaveFile(filePath);
				}
			}
			Gui::PopStyleColor();

			Gui::Separator();

			constexpr auto getBackendName = [](Audio::AudioBackend backend)
			{
				switch (backend)
				{
				case Audio::AudioBackend::WASAPIShared: return "WASAPI (Shared)";
				case Audio::AudioBackend::WASAPIExclusive: return "WASAPI (Exclusive)";
				default: return "Invalid";
				}
			};

			Gui::Text("GetAudioBackend(): %s", getBackendName(engine.GetAudioBackend()));

			if (selectedAudioBackend == Audio::AudioBackend::Invalid)
				selectedAudioBackend = engine.GetAudioBackend();

			if (Gui::BeginCombo("Audio API##Combo", getBackendName(selectedAudioBackend)))
			{
				for (size_t i = 0; i < EnumCount<Audio::AudioBackend>(); i++)
				{
					const auto backend = static_cast<Audio::AudioBackend>(i);
					if (backend != Audio::AudioBackend::Invalid && Gui::Selectable(getBackendName(backend), (selectedAudioBackend == backend)))
						selectedAudioBackend = backend;
				}

				Gui::EndCombo();
			}

			Gui::Separator();

			if (Gui::Button("SetAudioBackend()", vec2(Gui::CalcItemWidth(), 0.0f)))
				engine.SetAudioBackend(selectedAudioBackend);

			if (Gui::Button("ShowControlPanel()", vec2(Gui::CalcItemWidth(), 0.0f)))
				engine.DebugShowControlPanel();

			Gui::Separator();

			if (static_cast<int>(selectedMixingBehavior) < 0)
				selectedMixingBehavior = engine.GetChannelMixer().GetMixingBehavior();

			if (Gui::Combo("Mixing Behavior##Combo", reinterpret_cast<int*>(&selectedMixingBehavior), mixingBehaviorNames.data(), static_cast<int>(mixingBehaviorNames.size())))
				engine.GetChannelMixer().SetMixingBehavior(selectedMixingBehavior);
		}

		if (Gui::CollapsingHeader("Audio Voices"))
		{
			Gui::BeginChild("##AudioInstanceChilds", vec2(0.0f, audioInstancesChildHeight), true);
			{
				std::array<Audio::Voice, Audio::AudioEngine::MaxSimultaneousVoices> allVoices;
				size_t voiceCount;

				engine.DebugGetAllVoices(allVoices.data(), &voiceCount);

				for (size_t i = 0; i < voiceCount; i++)
				{
					const auto voice = allVoices[i];
					const auto voiceName = voice.GetName();
					const auto voiceSource = voice.GetSource();

					char sourceNameBuffer[32] = "SourceHandle::Invalid";
					if (voiceSource != Audio::SourceHandle::Invalid)
						sprintf_s(sourceNameBuffer, "SourceHandle { 0x%X }", static_cast<Audio::HandleBaseType>(voiceSource));

					Gui::TextDisabled("%.*s | (%s / %s) | (%d%%) | %s [%s]",
						static_cast<int>(voiceName.size()),
						voiceName.data(),
						voice.GetPosition().FormatTime().data(),
						voice.GetDuration().FormatTime().data(),
						static_cast<int>(voice.GetVolume() * 100.0f),
						voice.GetIsPlaying() ? "Play" : "Pause",
						sourceNameBuffer);
				}
			}
			Gui::EndChild();
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio File Test"))
		{
			checkStartStream();

			if (testSongSource == Audio::SourceHandle::Invalid)
				testSongSource = engine.LoadAudioSource(testSongPath);

			if (Gui::Button("AddVoice()"))
			{
				if (testSongVoice.IsValid())
					engine.RemoveVoice(testSongVoice);

				testSongVoice = engine.AddVoice(testSongSource, "AudioTestWindow::TestSongVoice", true);
			}

			if (testSongVoice != Audio::VoiceHandle::Invalid)
			{
				if (!testSongVoice.IsValid())
					return;

				Gui::Separator();

				f32 position = static_cast<f32>(testSongVoice.GetPosition().TotalSeconds());
				f32 duration = static_cast<f32>(testSongVoice.GetDuration().TotalSeconds());
				if (Gui::SliderFloat("TestSongVoice::Position", &position, 0, duration, "%f sec"))
					testSongVoice.SetPosition(TimeSpan::FromSeconds(position));

				Gui::Separator();
				Gui::Text("TestSongVoice::GetDuration(): %s", testSongVoice.GetDuration().FormatTime().data());
				Gui::Separator();
				Gui::Text("TestSongVoice::GetPosition(): %s", testSongVoice.GetPosition().FormatTime().data());
				Gui::Separator();

				f32 volume = testSongVoice.GetVolume();
				if (Gui::SliderFloat("Volume", &volume, Audio::AudioEngine::MinVolume, Audio::AudioEngine::MaxVolume))
					testSongVoice.SetVolume(volume);

				auto boolColorText = [&onColor, &offColor](const char* label, bool value)
				{
					Gui::Text(label); Gui::SameLine();
					Gui::TextColored(value ? onColor : offColor, value ? "true" : "false");
				};

				boolColorText("TestSongVoice::GetIsPlaying(): ", testSongVoice.GetIsPlaying());

				bool isPlaying = testSongVoice.GetIsPlaying();
				if (Gui::Checkbox("testSongVoice::IsPlaying", &isPlaying))
					testSongVoice.SetIsPlaying(isPlaying);

				bool isLooping = testSongVoice.GetIsLooping();
				if (Gui::Checkbox("TestSongVoice::IsLooping", &isLooping))
					testSongVoice.SetIsLooping(isLooping);
			}
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Button Sound Test"))
		{
			checkStartStream();

			if (buttonTestSource == Audio::SourceHandle::Invalid)
				buttonTestSource = engine.LoadAudioSource(testButtonSoundPath);

			Gui::Button("PlaySound(TestButtonSource)");
			bool addButtonSound = Gui::IsItemHovered() && Gui::IsMouseClicked(0);

			Gui::SliderFloat("Button Volume", &testButtonVolume, Audio::AudioEngine::MinVolume, Audio::AudioEngine::MaxVolume);

			if (Gui::IsWindowFocused())
			{
				static constexpr std::array keys =
				{
					Input::KeyCode_W,
					Input::KeyCode_W,
					Input::KeyCode_A,
					Input::KeyCode_S,
					Input::KeyCode_D,
					Input::KeyCode_I,
					Input::KeyCode_J,
					Input::KeyCode_K,
					Input::KeyCode_L,
				};

				for (const auto& keyCode : keys)
					addButtonSound |= Input::Keyboard::IsTapped(keyCode);

				static constexpr std::array buttons =
				{
					Input::DS4Button::DPad_Up,
					Input::DS4Button::DPad_Down,
					Input::DS4Button::DPad_Left,
					Input::DS4Button::DPad_Right,
					Input::DS4Button::Triangle,
					Input::DS4Button::Circle,
					Input::DS4Button::Cross,
					Input::DS4Button::Square,
					Input::DS4Button::L_Trigger,
					Input::DS4Button::R_Trigger,
				};

				for (const auto& button : buttons)
					addButtonSound |= Input::DualShock4::IsTapped(button);
			}

			if (addButtonSound)
				engine.PlaySound(buttonTestSource, "AudioTestWindow::TestButtonSound", testButtonVolume);
		}
		Gui::Separator();
	}

	void AudioTestWindow::RefreshDeviceInfoList()
	{
		auto& engine = Audio::AudioEngine::GetInstance();
#if 0
		size_t deviceCount = engine.GetDeviceCount();

		deviceInfoList.clear();
		deviceInfoList.reserve(deviceCount);

		for (int i = 0; i < deviceCount; i++)
		{
			deviceInfoList.push_back({ engine.GetDeviceInfo(i) });

			std::stringstream stringStream;
			{
				auto sampleRates = &deviceInfoList[i].Info.sampleRates;
				for (size_t j = 0; j < sampleRates->size(); j++)
				{
					stringStream << sampleRates->at(j);
					if (j < sampleRates->size() - 1)
						stringStream << ", ";
				}

				deviceInfoList[i].SampleRatesString = stringStream.str();
			}
			stringStream.str("");
			{
				auto nativeFormats = deviceInfoList[i].Info.nativeFormats;

				size_t totalContainedFlags = 0;

				for (const auto&[format, name] : audioFormatDescriptions)
				{
					if ((nativeFormats & format) != 0)
						totalContainedFlags++;
				}

				size_t flagStringsAdded = 0;
				for (const auto&[format, name] : audioFormatDescriptions)
				{
					if ((nativeFormats & format) != 0)
					{
						flagStringsAdded++;
						stringStream << name;

						if (flagStringsAdded != totalContainedFlags)
							stringStream << ", ";
					}
				}

				deviceInfoList[i].NativeFormatsString = stringStream.str();
			}
		}
#endif
	}

	void AudioTestWindow::ShowDeviceInfoProperties(const ExtendedDeviceInfo& deviceInfo, int uid)
	{
		Gui::PushID(uid);

#if 0
		Gui::AlignTextToFramePadding();

		bool nodeOpen = Gui::TreeNode("DeviceInfo", "%s", deviceInfo.Info.name.c_str(), uid);

		Gui::NextColumn();
		Gui::AlignTextToFramePadding();
		Gui::Text("Properties");
		Gui::NextColumn();

		if (!nodeOpen)
		{
			Gui::PopID();
			return;
		}

		for (int i = 0; i < deviceInfoFieldNames.size(); i++)
		{
			Gui::PushID(i);
			{
				Gui::AlignTextToFramePadding();
				Gui::TreeNodeEx("DeviceInfoField", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "%s", deviceInfoFieldNames[i]);
				Gui::NextColumn();
				Gui::PushItemWidth(-1);

				const char* identifierLabel = "##value";

				switch (i)
				{
				case 0: Gui::InputText(identifierLabel, (char*)deviceInfo.Info.name.c_str(), deviceInfo.Info.name.length(), ImGuiInputTextFlags_ReadOnly); break;
				case 1: Gui::InputInt(identifierLabel, (int*)&deviceInfo.Info.outputChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
				case 2: Gui::InputInt(identifierLabel, (int*)&deviceInfo.Info.inputChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
				case 3: Gui::InputInt(identifierLabel, (int*)&deviceInfo.Info.duplexChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
				case 4: Gui::Text(deviceInfo.Info.isDefaultOutput ? "True" : "False"); break;
				case 5: Gui::Text(deviceInfo.Info.isDefaultInput ? "True" : "False"); break;
				case 6: Gui::TextWrapped(deviceInfo.SampleRatesString.c_str()); break;
				case 7: Gui::TextWrapped(deviceInfo.NativeFormatsString.c_str()); break;
				}

				Gui::PopItemWidth();
				Gui::NextColumn();
			}
			Gui::PopID();
		}

		Gui::TreePop();
#endif

		Gui::PopID();
	}
}
