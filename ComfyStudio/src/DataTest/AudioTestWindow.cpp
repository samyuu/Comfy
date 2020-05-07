#include "AudioTestWindow.h"
#include "Core/Application.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include "Time/TimeUtilities.h"
#include "IO/Path.h"
#include <numeric>

namespace Comfy::DataTest
{
	AudioTestWindow::AudioTestWindow(Application* parent) : BaseWindow(parent)
	{
		CloseWindow();
	}

	AudioTestWindow::~AudioTestWindow()
	{
	}

	void AudioTestWindow::DrawGui()
	{
		auto& engine = Audio::Engine::GetInstance();

		auto checkStartStream = [&]()
		{
			static bool checkStartStream = true;
			if (checkStartStream)
			{
				engine.EnsureStreamRunning();
				checkStartStream = false;
			}
		};

		Gui::TextUnformatted("Audio Test:");
		Gui::Separator();

		float masterVolume = engine.GetMasterVolume();
		if (Gui::SliderFloat("Master Volume", &masterVolume, Audio::Engine::MinVolume, Audio::Engine::MaxVolume))
			engine.SetMasterVolume(masterVolume);
		Gui::Separator();

		if (Gui::CollapsingHeader("Device List"))
		{
			Gui::Text("Device(s): %d", static_cast<int>(deviceInfoList.size()));

			float halfWindowWidth = Gui::GetWindowWidth() / 2.0f;

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
			const auto buttonSize = vec2(Gui::GetWindowWidth() / 4.0f, 0.0);

			Gui::PushItemWidth(buttonSize.x);
			{
				if (Gui::Button("OpenStream()", buttonSize))
					engine.OpenStream();
				Gui::SameLine();
				if (Gui::Button("CloseStream()", buttonSize))
					engine.CloseStream();

				bool isStreamOpen = engine.GetIsStreamOpen();

				Gui::SameLine();
				Gui::Text("%-24s:", "GetIsStreamOpen()");
				Gui::SameLine();
				Gui::TextColored(isStreamOpen ? onColor : offColor, isStreamOpen ? "Open" : "Closed");
			}
			{
				if (Gui::Button("StartStream()", buttonSize))
					engine.StartStream();
				Gui::SameLine();
				if (Gui::Button("StopStream()", buttonSize))
					engine.StopStream();

				bool isStreamRunning = engine.GetIsStreamRunning();

				Gui::SameLine();
				Gui::Text("%-21s:", "GetIsStreamRunning()");
				Gui::SameLine();
				Gui::TextColored(isStreamRunning ? onColor : offColor, isStreamRunning ? "Running" : "Stopped");
			}
			Gui::PopItemWidth();

			Gui::Text("%-25s: %u", "GetChannelCount()", engine.GetChannelCount());
			Gui::Text("%-28s: %u Hz", "GetSampleRate()", engine.GetSampleRate());
			Gui::Text("%-28s: %.3f sec", "GetStreamTime()", engine.GetStreamTime().TotalSeconds());
			Gui::Text("%-23s: %.3f ms", "GetCallbackFrequency()", engine.GetCallbackFrequency().TotalMilliseconds());

			const auto durations = engine.DebugGetCallbackDurations();

			std::array<float, durations.size()> durationsMS;
			for (size_t i = 0; i < durations.size(); i++)
				durationsMS[i] = static_cast<float>(durations[i].TotalMilliseconds());

			const float totalBufferProcessTimeMS = std::accumulate(durationsMS.begin(), durationsMS.end(), 0.0f);
			const float averageProcessTimeMS = (totalBufferProcessTimeMS / static_cast<float>(durationsMS.size()));

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
					std::array<float, lastPlayedSamples[0].size()> normalizedSamples;

					for (size_t sample = 0; sample < normalizedSamples.size(); sample++)
						normalizedSamples[sample] = static_cast<float>(lastPlayedSamples[channel][sample]) / static_cast<float>(std::numeric_limits<i16>::max());

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

			newBufferFrameCount = std::clamp(newBufferFrameCount, Audio::Engine::MinBufferFrameCount, Audio::Engine::MaxBufferFrameCount);

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

			constexpr auto getAudioAPIName = [](Audio::Engine::AudioAPI api)
			{
				switch (api)
				{
				case Audio::Engine::AudioAPI::ASIO:
					return "AudioAPI::ASIO";
				case Audio::Engine::AudioAPI::WASAPI:
					return "AudioAPI::WASAPI";
				default:
					return "AudioAPI::Invalid";
				}
			};

			Gui::Text("GetAudioAPI(): %s", getAudioAPIName(engine.GetAudioAPI()));

			if (selectedAudioAPI == Audio::Engine::AudioAPI::Invalid)
				selectedAudioAPI = engine.GetAudioAPI();

			if (Gui::BeginCombo("Audio API##Combo", getAudioAPIName(selectedAudioAPI)))
			{
				for (const auto availableAPI : std::array { Audio::Engine::AudioAPI::WASAPI, Audio::Engine::AudioAPI::ASIO })
				{
					if (Gui::Selectable(getAudioAPIName(availableAPI), (selectedAudioAPI == availableAPI)))
						selectedAudioAPI = availableAPI;
				}

				Gui::EndCombo();
			}

			Gui::Separator();

			if (Gui::Button("SetAudioAPI()", vec2(Gui::CalcItemWidth(), 0.0f)))
				engine.SetAudioAPI(selectedAudioAPI);

			if (Gui::Button("ShowControlPanel()", vec2(Gui::CalcItemWidth(), 0.0f)))
				engine.DebugShowControlPanel();

			Gui::Separator();

			if (static_cast<int>(selectedMixingBehavior) < 0)
				selectedMixingBehavior = engine.GetChannelMixer().GetMixingBehavior();

			if (Gui::Combo("Mixing Behavior##Combo", reinterpret_cast<int*>(&selectedMixingBehavior), mixingBehaviorNames.data(), static_cast<int>(mixingBehaviorNames.size())))
				engine.GetChannelMixer().SetMixingBehavior(selectedMixingBehavior);
		}

		/*
		if (Gui::CollapsingHeader("Audio Data"))
		{
			Gui::TextDisabled("(Dummy)");
			//Gui::BeginCombo();
			//Gui::PlotLines("PlitPLines", );

			// static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
			// Gui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

			//float floatBuffer[MAX_BUFFER_SIZE];
			//for (size_t i = 0; i < engine.GetBufferSize(); i++)
			//	floatBuffer[i] = (float)(engine.SAMPLE_BUFFER_PTR[i]);
			//Gui::PlotLines("SAMPLE_BUFFER_PTR", floatBuffer, engine.GetBufferSize());
		}
		Gui::Separator();
		*/

		if (Gui::CollapsingHeader("Audio Voices"))
		{
			Gui::BeginChild("##AudioInstanceChilds", vec2(0.0f, audioInstancesChildHeight), true);
			{
				std::array<Audio::Voice, Audio::Engine::MaxSimultaneousVoices> allVoices;
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
						voice.GetPosition().ToString().c_str(),
						voice.GetDuration().ToString().c_str(),
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

				float position = static_cast<float>(testSongVoice.GetPosition().TotalSeconds());
				float duration = static_cast<float>(testSongVoice.GetDuration().TotalSeconds());
				if (Gui::SliderFloat("TestSongVoice::Position", &position, 0, duration, "%f sec"))
					testSongVoice.SetPosition(TimeSpan::FromSeconds(position));

				Gui::Separator();
				Gui::Text("TestSongVoice::GetDuration(): %s", testSongVoice.GetDuration().ToString().c_str());
				Gui::Separator();
				Gui::Text("TestSongVoice::GetPosition(): %s", testSongVoice.GetPosition().ToString().c_str());
				Gui::Separator();

				float volume = testSongVoice.GetVolume();
				if (Gui::SliderFloat("Volume", &volume, Audio::Engine::MinVolume, Audio::Engine::MaxVolume))
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

			Gui::SliderFloat("Button Volume", &testButtonVolume, Audio::Engine::MinVolume, Audio::Engine::MaxVolume);

			if (Gui::IsWindowFocused())
			{
				static constexpr std::array keys =
				{
					KeyCode_W,
					KeyCode_W,
					KeyCode_A,
					KeyCode_S,
					KeyCode_D,
					KeyCode_I,
					KeyCode_J,
					KeyCode_K,
					KeyCode_L,
				};

				for (const auto& keyCode : keys)
					addButtonSound |= Keyboard::IsTapped(keyCode);

				static constexpr std::array buttons =
				{
					Ds4Button::DPad_Up,
					Ds4Button::DPad_Down,
					Ds4Button::DPad_Left,
					Ds4Button::DPad_Right,
					Ds4Button::Triangle,
					Ds4Button::Circle,
					Ds4Button::Cross,
					Ds4Button::Square,
					Ds4Button::L_Trigger,
					Ds4Button::R_Trigger,
				};

				for (const auto& button : buttons)
					addButtonSound |= DualShock4::IsTapped(button);
			}

			if (addButtonSound)
				engine.PlaySound(buttonTestSource, "AudioTestWindow::TestButtonSound", testButtonVolume);
		}
		Gui::Separator();
	}

	const char* AudioTestWindow::GetGuiName() const
	{
		return "Audio Test";
	}

	ImGuiWindowFlags AudioTestWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AudioTestWindow::RefreshDeviceInfoList()
	{
		auto& engine = Audio::Engine::GetInstance();
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
