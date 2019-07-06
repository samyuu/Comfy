#include "AudioTestWindow.h"
#include <sstream>
#include <memory>
#include "Application.h"
#include "Audio/AudioInstance.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"

AudioTestWindow::AudioTestWindow(Application* parent) : BaseWindow(parent)
{
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
		ImGui::ShowDemoWindow(nullptr);

	ImGui::Text("AUDIO TEST:");
	ImGui::Separator();

	float masterVolume = engine->GetMasterVolume();
	if (ImGui::SliderFloat("Master Volume", &masterVolume, MIN_VOLUME, MAX_VOLUME))
		engine->SetMasterVolume(masterVolume);
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Device List"))
	{
		ImGui::Text("Device(s): %d", deviceInfoList.size());

		float halfWindowWidth = ImGui::GetWindowWidth() / 2.0f;

		ImGui::SameLine(halfWindowWidth);
		if (ImGui::Button("Refresh Device List", ImVec2(halfWindowWidth, 0)))
			RefreshDeviceInfoList();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);

		for (size_t i = 0; i < deviceInfoList.size(); i++)
			ShowDeviceInfoProperties(deviceInfoList.at(i), i);

		ImGui::Columns(1);
		ImGui::PopStyleVar();
	}
	ImGui::Separator();

	const ImVec4 onColor = ImVec4(0.14f, 0.78f, 0.21f, 1.00f);
	const ImVec4 offColor = ImVec4(0.95f, 0.12f, 0.12f, 1.00f);

	if (ImGui::CollapsingHeader("Stream Control"))
	{
		const ImVec2 buttonSize(ImGui::GetWindowWidth() / 4.f, 0);

		ImGui::PushItemWidth(buttonSize.x);
		{
			if (ImGui::Button("engine->OpenStream()", buttonSize))
				engine->OpenStream();
			ImGui::SameLine();
			if (ImGui::Button("engine->CloseStream()", buttonSize))
				engine->CloseStream();

			bool isStreamOpen = engine->GetIsStreamOpen();

			ImGui::SameLine();
			ImGui::Text("engine->GetIsStreamOpen():");
			ImGui::SameLine();
			ImGui::TextColored(isStreamOpen ? onColor : offColor, isStreamOpen ? "open" : "closed");
		}

		{
			if (ImGui::Button("engine->StartStream()", buttonSize))
				engine->StartStream();
			ImGui::SameLine();
			if (ImGui::Button("engine->StopStream()", buttonSize))
				engine->StopStream();

			bool isStreamRunning = engine->GetIsStreamRunning();

			ImGui::SameLine();
			ImGui::Text("engine->GetIsStreamRunning():");
			ImGui::SameLine();
			ImGui::TextColored(isStreamRunning ? onColor : offColor, isStreamRunning ? "running" : "stopped");
		}
		ImGui::PopItemWidth();

		ImGui::Text("engine->GetStreamTime(): %.3f s", engine->GetStreamTime());
		ImGui::Text("engine->GetCallbackLatency(): %.3f ms", engine->GetCallbackLatency() * 1000.0);
		ImGui::Text("engine->GetBufferSize(): %d", engine->GetBufferSize());
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Audio API"))
	{
		ImGui::TextDisabled("(engine->GetActiveAudioApi(): %s)", audioApiNames[engine->GetActiveAudioApi()]);

		if (selectedAudioApi == AUDIO_API_INVALID)
			selectedAudioApi = engine->GetActiveAudioApi();

		ImGui::Combo("Audio API##combo", (int*)&selectedAudioApi, audioApiNames.data(), audioApiNames.size());
		ImGui::Separator();

		if (ImGui::Button("engine->SetAudioApi()", ImVec2(ImGui::CalcItemWidth(), 0)))
			engine->SetAudioApi(selectedAudioApi);

		if (ImGui::Button("engine->ShowControlPanel()", ImVec2(ImGui::CalcItemWidth(), 0)))
			engine->ShowControlPanel();
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Audio Buffer"))
	{
		ImGui::TextDisabled("(engine->GetBufferSize(): %d)", engine->GetBufferSize());

		if (newBufferSize < 0)
			newBufferSize = engine->GetBufferSize();

		const uint32_t slowStep = 8, fastStep = 64;
		ImGui::InputScalar("Buffer Size", ImGuiDataType_U32, &newBufferSize, &slowStep, &fastStep, "%u");

		if (newBufferSize > MAX_BUFFER_SIZE)
			newBufferSize = MAX_BUFFER_SIZE;

		if (ImGui::Button("engine->SetBufferSize()", ImVec2(ImGui::CalcItemWidth(), 0)))
			engine->SetBufferSize(newBufferSize);
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Audio Data"))
	{
		ImGui::TextDisabled("(Dummy)");
		//ImGui::BeginCombo();
		//ImGui::PlotLines("PlitPLines", );

		// static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
		// ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

		//float floatBuffer[MAX_BUFFER_SIZE];
		//for (size_t i = 0; i < engine->GetBufferSize(); i++)
		//	floatBuffer[i] = (float)(engine->SAMPLE_BUFFER_PTR[i]);
		//ImGui::PlotLines("engine->SAMPLE_BUFFER_PTR", floatBuffer, engine->GetBufferSize());
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Audio Instances"))
	{
		ImGui::BeginChild("##audio_instances_child", ImVec2(0, audioInstancesChildHeight), true);
		if (!engine->callbackRunning)
		{
			for (auto &instance : engine->audioInstances)
			{
				if (instance == nullptr || instance->GetSampleProvider() == nullptr)
				{
					ImGui::TextDisabled("nullptr");
					continue;
				}

				auto playingString = instance->GetIsPlaying() ? "PLAY" : "PAUSE";

				ImGui::TextDisabled("%s | (%s / %s) | (%d%%) | %s",
					instance->GetName(),
					instance->GetPosition().FormatTime().c_str(),
					instance->GetDuration().FormatTime().c_str(),
					(int)(instance->GetVolume() * 100.0f),
					playingString);
			}
		}
		ImGui::EndChild();
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Audio File Test"))
	{
		checkStartStream();

		if (!songTestStream.GetIsInitialized())
			songTestStream.LoadFromFile(testSongPath);

		if (ImGui::Button("engine->AddAudioInstance()"))
		{
			if (songAudioInstance != nullptr)
				songAudioInstance->SetAppendRemove(true);

			songAudioInstance = std::make_shared<AudioInstance>(&songTestStream, true, "AudioTestWindow::TestSongInstance");
			engine->AddAudioInstance(songAudioInstance);
		}

		if (songAudioInstance != nullptr)
		{
			if (songAudioInstance->GetHasBeenRemoved())
				return;

			AudioInstance* audioInstance = songAudioInstance.get();

			ImGui::Separator();
			ImGui::Text("audioInstance->GetChannelCount(): %u", songTestStream.GetChannelCount());
			ImGui::Text("audioInstance->GetSampleCount(): %u", songTestStream.GetSampleCount());
			ImGui::Text("audioInstance->GetSampleRate(): %u", songTestStream.GetSampleRate());

			int samplePosition = audioInstance->GetSamplePosition();
			if (ImGui::SliderInt("audioInstance->SamplePosition", &samplePosition, 0, audioInstance->GetSampleCount()))
				audioInstance->SetSamplePosition(samplePosition);

			float position = audioInstance->GetPosition().TotalSeconds();
			if (ImGui::SliderFloat("audioInstance->Position", &position, 0, audioInstance->GetDuration().TotalSeconds(), "%f"))
				audioInstance->SetPosition(TimeSpan::FromSeconds(position));

			ImGui::Separator();
			ImGui::Text("audioInstance->GetDuration(): %s", audioInstance->GetDuration().FormatTime().c_str());
			ImGui::Separator();
			ImGui::Text("audioInstance->GetPosition(): %s", audioInstance->GetPosition().FormatTime().c_str());
			ImGui::Separator();

			float volume = audioInstance->GetVolume();
			if (ImGui::SliderFloat("volume", &volume, MIN_VOLUME, MAX_VOLUME))
				audioInstance->SetVolume(volume);

			auto boolColorText = [&onColor, &offColor](const char* label, bool value)
			{
				ImGui::Text(label); ImGui::SameLine();
				ImGui::TextColored(value ? onColor : offColor, value ? "true" : "false");
			};

			boolColorText("audioInstance->GetIsPlaying(): ", audioInstance->GetIsPlaying());
			boolColorText("audioInstance->GetHasReachedEnd(): ", audioInstance->GetHasReachedEnd());
			boolColorText("audioInstance->GetHasBeenRemoved(): ", audioInstance->GetHasBeenRemoved());

			bool isPlaying = audioInstance->GetIsPlaying();
			if (ImGui::Checkbox("audioInstance->IsPlaying", &isPlaying))
				audioInstance->SetIsPlaying(isPlaying);

			bool isLooping = audioInstance->GetIsLooping();
			if (ImGui::Checkbox("audioInstance->IsLooping", &isLooping))
				audioInstance->SetIsLooping(isLooping);
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Button Sound Test"))
	{
		checkStartStream();

		if (!testButtonSound.GetIsInitialized())
			testButtonSound.LoadFromFile("rom/sound/button/01_button1.wav");

		ImGui::Button("PlaySound(TestButtonSound)");
		bool addButtonSound = ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);

		ImGui::SliderFloat("Button Volume", &testButtonVolume, MIN_VOLUME, MAX_VOLUME);

		if (ImGui::IsWindowFocused())
		{
			short keys[] = { 'W', 'A', 'S', 'D', 'I', 'J', 'K', 'L' };
			for (size_t i = 0; i < IM_ARRAYSIZE(keys); i++)
				addButtonSound |= Keyboard::IsTapped(keys[i]);

			Ds4Button buttons[] = { DS4_DPAD_UP, DS4_DPAD_DOWN, DS4_DPAD_LEFT, DS4_DPAD_RIGHT, DS4_TRIANGLE, DS4_CIRCLE, DS4_CROSS, DS4_SQUARE, DS4_L_TRIGGER, DS4_R_TRIGGER };
			for (size_t i = 0; i < IM_ARRAYSIZE(buttons); i++)
				addButtonSound |= DualShock4::IsTapped(buttons[i]);
		}

		if (addButtonSound)
			engine->PlaySound(&testButtonSound, testButtonVolume, "AudioTestWindow::TestButtonSound");
	}
	ImGui::Separator();
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
	auto engine = AudioEngine::GetInstance();
	int deviceCount = engine->GetDeviceCount();

	deviceInfoList.clear();
	deviceInfoList.reserve(deviceCount);

	for (size_t i = 0; i < deviceCount; i++)
	{
		deviceInfoList.push_back({ engine->GetDeviceInfo(i) });

		std::stringstream stringStream;
		{
			auto sampleRates = &deviceInfoList[i].Info.sampleRates;
			for (size_t i = 0; i < sampleRates->size(); i++)
			{
				stringStream << sampleRates->at(i);
				if (i < sampleRates->size() - 1)
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
	// Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
	ImGui::PushID(uid);

	// Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
	ImGui::AlignTextToFramePadding();

	bool nodeOpen = ImGui::TreeNode("DeviceInfo", "%s", deviceInfo.Info.name.c_str(), uid);

	ImGui::NextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Properties");
	ImGui::NextColumn();

	if (!nodeOpen)
	{
		ImGui::PopID();
		return;
	}

	for (int i = 0; i < deviceInfoFieldNames.size(); i++)
	{
		ImGui::PushID(i);
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TreeNodeEx("DeviceInfoField", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "%s", deviceInfoFieldNames[i]);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			const char* identifierLabel = "##value";

			switch (i)
			{
			case 0: ImGui::InputText(identifierLabel, (char*)deviceInfo.Info.name.c_str(), deviceInfo.Info.name.length(), ImGuiInputTextFlags_ReadOnly); break;
			case 1: ImGui::InputInt(identifierLabel, (int*)&deviceInfo.Info.outputChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
			case 2: ImGui::InputInt(identifierLabel, (int*)&deviceInfo.Info.inputChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
			case 3: ImGui::InputInt(identifierLabel, (int*)&deviceInfo.Info.duplexChannels, 1, 4, ImGuiInputTextFlags_ReadOnly); break;
			case 4: ImGui::Text(deviceInfo.Info.isDefaultOutput ? "true" : "false"); break;
			case 5: ImGui::Text(deviceInfo.Info.isDefaultInput ? "true" : "false"); break;
			case 6: ImGui::TextWrapped(deviceInfo.SampleRatesString.c_str()); break;
			case 7: ImGui::TextWrapped(deviceInfo.NativeFormatsString.c_str()); break;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();
		}
		ImGui::PopID();
	}

	ImGui::TreePop();

	ImGui::PopID();
}
