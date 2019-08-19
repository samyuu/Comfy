#include "AudioTestWindow.h"
#include "Core/Application.h"
#include "Audio/AudioInstance.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include <sstream>

namespace DataTest
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
		AudioEngine* engine = AudioEngine::GetInstance();

		auto checkStartStream = [&engine]()
		{
			static bool checkStartStream = true;
			if (checkStartStream)
			{
				if (!engine->GetIsStreamOpen())
					engine->OpenStream();
				if (!engine->GetIsStreamRunning())
					engine->StartStream();
				checkStartStream = false;
			}
		};

		if (false)
			Gui::ShowDemoWindow(nullptr);

		Gui::Text("Audio Test:");
		Gui::Separator();

		float masterVolume = engine->GetMasterVolume();
		if (Gui::SliderFloat("Master Volume", &masterVolume, MIN_VOLUME, MAX_VOLUME))
			engine->SetMasterVolume(masterVolume);
		Gui::Separator();

		if (Gui::CollapsingHeader("Device List"))
		{
			Gui::Text("Device(s): %d", deviceInfoList.size());

			float halfWindowWidth = Gui::GetWindowWidth() / 2.0f;

			Gui::SameLine(halfWindowWidth);
			if (Gui::Button("Refresh Device List", ImVec2(halfWindowWidth, 0)))
				RefreshDeviceInfoList();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			Gui::Columns(2);

			for (size_t i = 0; i < deviceInfoList.size(); i++)
				ShowDeviceInfoProperties(deviceInfoList.at(i), static_cast<int>(i));

			Gui::Columns(1);
			Gui::PopStyleVar();
		}
		Gui::Separator();

		const ImVec4 onColor = ImVec4(0.14f, 0.78f, 0.21f, 1.00f);
		const ImVec4 offColor = ImVec4(0.95f, 0.12f, 0.12f, 1.00f);

		if (Gui::CollapsingHeader("Stream Control"))
		{
			const ImVec2 buttonSize(Gui::GetWindowWidth() / 4.f, 0);

			Gui::PushItemWidth(buttonSize.x);
			{
				if (Gui::Button("engine->OpenStream()", buttonSize))
					engine->OpenStream();
				Gui::SameLine();
				if (Gui::Button("engine->CloseStream()", buttonSize))
					engine->CloseStream();

				bool isStreamOpen = engine->GetIsStreamOpen();

				Gui::SameLine();
				Gui::Text("engine->GetIsStreamOpen():");
				Gui::SameLine();
				Gui::TextColored(isStreamOpen ? onColor : offColor, isStreamOpen ? "open" : "closed");
			}

			{
				if (Gui::Button("engine->StartStream()", buttonSize))
					engine->StartStream();
				Gui::SameLine();
				if (Gui::Button("engine->StopStream()", buttonSize))
					engine->StopStream();

				bool isStreamRunning = engine->GetIsStreamRunning();

				Gui::SameLine();
				Gui::Text("engine->GetIsStreamRunning():");
				Gui::SameLine();
				Gui::TextColored(isStreamRunning ? onColor : offColor, isStreamRunning ? "running" : "stopped");
			}
			Gui::PopItemWidth();

			Gui::Text("engine->GetStreamTime(): %.3f s", engine->GetStreamTime());
			Gui::Text("engine->GetCallbackLatency(): %.3f ms", engine->GetCallbackLatency() * 1000.0);
			Gui::Text("engine->GetBufferSize(): %d", engine->GetBufferSize());
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio API"))
		{
			Gui::TextDisabled("(engine->GetActiveAudioApi(): %s)", audioApiNames[static_cast<int>(engine->GetActiveAudioApi())]);

			if (selectedAudioApi == AudioApi::Invalid)
				selectedAudioApi = engine->GetActiveAudioApi();

			Gui::Combo("Audio API##combo", reinterpret_cast<int*>(&selectedAudioApi), audioApiNames.data(), static_cast<int>(audioApiNames.size()));
			Gui::Separator();

			if (Gui::Button("engine->SetAudioApi()", ImVec2(Gui::CalcItemWidth(), 0)))
				engine->SetAudioApi(selectedAudioApi);

			if (Gui::Button("engine->ShowControlPanel()", ImVec2(Gui::CalcItemWidth(), 0)))
				engine->ShowControlPanel();
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio Buffer"))
		{
			Gui::TextDisabled("(engine->GetBufferSize(): %d)", engine->GetBufferSize());

			if (newBufferSize < 0)
				newBufferSize = engine->GetBufferSize();

			const uint32_t slowStep = 8, fastStep = 64;
			Gui::InputScalar("Buffer Size", ImGuiDataType_U32, &newBufferSize, &slowStep, &fastStep, "%u");

			if (newBufferSize > MAX_BUFFER_SIZE)
				newBufferSize = MAX_BUFFER_SIZE;

			if (Gui::Button("engine->SetBufferSize()", ImVec2(Gui::CalcItemWidth(), 0)))
				engine->SetBufferSize(newBufferSize);
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio Data"))
		{
			Gui::TextDisabled("(Dummy)");
			//Gui::BeginCombo();
			//Gui::PlotLines("PlitPLines", );

			// static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
			// Gui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

			//float floatBuffer[MAX_BUFFER_SIZE];
			//for (size_t i = 0; i < engine->GetBufferSize(); i++)
			//	floatBuffer[i] = (float)(engine->SAMPLE_BUFFER_PTR[i]);
			//Gui::PlotLines("engine->SAMPLE_BUFFER_PTR", floatBuffer, engine->GetBufferSize());
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio Instances"))
		{
			Gui::BeginChild("##audio_instances_child", ImVec2(0, audioInstancesChildHeight), true);
			if (!engine->callbackRunning)
			{
				for (auto &instance : engine->audioInstances)
				{
					if (instance == nullptr || instance->GetSampleProvider() == nullptr)
					{
						Gui::TextDisabled("nullptr");
						continue;
					}

					auto playingString = instance->GetIsPlaying() ? "PLAY" : "PAUSE";

					Gui::TextDisabled("%s | (%s / %s) | (%d%%) | %s",
						instance->GetName(),
						instance->GetPosition().FormatTime().c_str(),
						instance->GetDuration().FormatTime().c_str(),
						(int)(instance->GetVolume() * 100.0f),
						playingString);
				}
			}
			Gui::EndChild();
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Audio File Test"))
		{
			checkStartStream();

			if (!songTestStream.GetIsInitialized())
				songTestStream.LoadFromFile(testSongPath);

			if (Gui::Button("engine->AddAudioInstance()"))
			{
				if (songAudioInstance != nullptr)
					songAudioInstance->SetAppendRemove(true);

				songAudioInstance = MakeRefPtr<AudioInstance>(&songTestStream, true, "AudioTestWindow::TestSongInstance");
				engine->AddAudioInstance(songAudioInstance);
			}

			if (songAudioInstance != nullptr)
			{
				if (songAudioInstance->GetHasBeenRemoved())
					return;

				AudioInstance* audioInstance = songAudioInstance.get();

				Gui::Separator();
				Gui::Text("audioInstance->GetChannelCount(): %u", songTestStream.GetChannelCount());
				Gui::Text("audioInstance->GetSampleCount(): %u", songTestStream.GetSampleCount());
				Gui::Text("audioInstance->GetSampleRate(): %u", songTestStream.GetSampleRate());

				int samplePosition = static_cast<int>(audioInstance->GetSamplePosition());
				int sampleCount = static_cast<int>(audioInstance->GetSampleCount());
				if (Gui::SliderInt("audioInstance->SamplePosition", &samplePosition, 0, sampleCount))
					audioInstance->SetSamplePosition(samplePosition);

				float position = static_cast<float>(audioInstance->GetPosition().TotalSeconds());
				float duration = static_cast<float>(audioInstance->GetDuration().TotalSeconds());
				if (Gui::SliderFloat("audioInstance->Position", &position, 0, duration, "%f"))
					audioInstance->SetPosition(TimeSpan::FromSeconds(position));

				Gui::Separator();
				Gui::Text("audioInstance->GetDuration(): %s", audioInstance->GetDuration().FormatTime().c_str());
				Gui::Separator();
				Gui::Text("audioInstance->GetPosition(): %s", audioInstance->GetPosition().FormatTime().c_str());
				Gui::Separator();

				float volume = audioInstance->GetVolume();
				if (Gui::SliderFloat("volume", &volume, MIN_VOLUME, MAX_VOLUME))
					audioInstance->SetVolume(volume);

				auto boolColorText = [&onColor, &offColor](const char* label, bool value)
				{
					Gui::Text(label); Gui::SameLine();
					Gui::TextColored(value ? onColor : offColor, value ? "true" : "false");
				};

				boolColorText("audioInstance->GetIsPlaying(): ", audioInstance->GetIsPlaying());
				boolColorText("audioInstance->GetHasReachedEnd(): ", audioInstance->GetHasReachedEnd());
				boolColorText("audioInstance->GetHasBeenRemoved(): ", audioInstance->GetHasBeenRemoved());

				bool isPlaying = audioInstance->GetIsPlaying();
				if (Gui::Checkbox("audioInstance->IsPlaying", &isPlaying))
					audioInstance->SetIsPlaying(isPlaying);

				bool isLooping = audioInstance->GetIsLooping();
				if (Gui::Checkbox("audioInstance->IsLooping", &isLooping))
					audioInstance->SetIsLooping(isLooping);
			}
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Button Sound Test"))
		{
			checkStartStream();

			if (!testButtonSound.GetIsInitialized())
				testButtonSound.LoadFromFile("rom/sound/button/01_button1.wav");

			Gui::Button("PlaySound(TestButtonSound)");
			bool addButtonSound = Gui::IsItemHovered() && Gui::IsMouseClicked(0);

			Gui::SliderFloat("Button Volume", &testButtonVolume, MIN_VOLUME, MAX_VOLUME);

			if (Gui::IsWindowFocused())
			{
				static KeyCode keys[] =
				{
					'W',
					'A',
					'S',
					'D',
					'I',
					'J',
					'K',
					'L'
				};
				for (size_t i = 0; i < IM_ARRAYSIZE(keys); i++)
					addButtonSound |= Keyboard::IsTapped(keys[i]);

				static Ds4Button buttons[] =
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
				for (size_t i = 0; i < IM_ARRAYSIZE(buttons); i++)
					addButtonSound |= DualShock4::IsTapped(buttons[i]);
			}

			if (addButtonSound)
				engine->PlaySound(&testButtonSound, testButtonVolume, "AudioTestWindow::TestButtonSound");
		}
		Gui::Separator();
	}

	const char* AudioTestWindow::GetGuiName() const
	{
		return u8"Audio Test";
	}

	ImGuiWindowFlags AudioTestWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AudioTestWindow::RefreshDeviceInfoList()
	{
		AudioEngine* engine = AudioEngine::GetInstance();
		size_t deviceCount = engine->GetDeviceCount();

		deviceInfoList.clear();
		deviceInfoList.reserve(deviceCount);

		for (int i = 0; i < deviceCount; i++)
		{
			deviceInfoList.push_back({ engine->GetDeviceInfo(i) });

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

				for (size_t i = 0; i < IM_ARRAYSIZE(audioFormatDescriptions); i++)
				{
					if ((nativeFormats & audioFormatDescriptions[i].Format) != 0)
						totalContainedFlags++;
				}

				size_t flagStringsAdded = 0;
				for (size_t i = 0; i < IM_ARRAYSIZE(audioFormatDescriptions); i++)
				{
					if ((nativeFormats & audioFormatDescriptions[i].Format) != 0)
					{
						flagStringsAdded++;
						stringStream << audioFormatDescriptions[i].Name;

						if (flagStringsAdded != totalContainedFlags)
							stringStream << ", ";
					}
				}

				deviceInfoList[i].NativeFormatsString = stringStream.str();
			}
		}
	}

	void AudioTestWindow::ShowDeviceInfoProperties(ExtendedDeviceInfo& deviceInfo, int uid)
	{
		Gui::PushID(uid);

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
				case 4: Gui::Text(deviceInfo.Info.isDefaultOutput ? "true" : "false"); break;
				case 5: Gui::Text(deviceInfo.Info.isDefaultInput ? "true" : "false"); break;
				case 6: Gui::TextWrapped(deviceInfo.SampleRatesString.c_str()); break;
				case 7: Gui::TextWrapped(deviceInfo.NativeFormatsString.c_str()); break;
				}

				Gui::PopItemWidth();
				Gui::NextColumn();
			}
			Gui::PopID();
		}

		Gui::TreePop();

		Gui::PopID();
	}
}