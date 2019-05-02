#include "AudioTestWindow.h"
#include <sstream>

AudioTestWindow::AudioTestWindow(Application* parent) : BaseWindow(parent)
{
}

AudioTestWindow::~AudioTestWindow()
{
}

void AudioTestWindow::DrawGui()
{
	AudioEngine* engine = AudioEngine::GetInstance();

	//ImGui::ShowDemoWindow(nullptr);

	ImGui::Text("AUDIO TEST:");
	ImGui::Separator();

	ImGui::SliderFloat("Master Volume", engine->GetMasterVolumePtr(), MIN_VOLUME, MAX_VOLUME);
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
		ImGui::Separator();
	}

	const ImVec4 onColor = ImVec4(0.14f, 0.78f, 0.21f, 1.00f);
	const ImVec4 offColor = ImVec4(0.95f, 0.12f, 0.12f, 1.00f);

	ImGui::Text("Stream Control");
	{
		if (ImGui::Button("engine->OpenAccess()"))
			engine->OpenAccess();
		ImGui::SameLine();
		if (ImGui::Button("engine->CloseAccess()"))
			engine->CloseAccess();

		bool isStreamOpen = engine->GetIsStreamOpen();

		ImGui::SameLine();
		ImGui::Text("engine->GetIsStreamOpen():        ");
		ImGui::SameLine();
		ImGui::TextColored(isStreamOpen ? onColor : offColor, isStreamOpen ? "open" : "closed");
	}
	{
		if (ImGui::Button("engine->StartStream()"))
			engine->StartStream();
		ImGui::SameLine();
		if (ImGui::Button("engine->StopStream()"))
			engine->StopStream();

		bool isStreamRunning = engine->GetIsStreamRunning();

		ImGui::SameLine();
		ImGui::Text("engine->GetIsStreamRunning(): ");
		ImGui::SameLine();
		ImGui::TextColored(isStreamRunning ? onColor : offColor, isStreamRunning ? "running" : "stopped");
	}

	ImGui::Separator();

	ImGui::Text("Audio API");
	{
		ImGui::TextDisabled("(engine->GetActiveAudioApi(): %s)", audioApiNames[engine->GetActiveAudioApi()]);

		if (selectedAudioApi == AUDIO_API_INVALID)
			selectedAudioApi = engine->GetActiveAudioApi();

		ImGui::Combo("Audio API##combo", (int*)&selectedAudioApi, audioApiNames, IM_ARRAYSIZE(audioApiNames));
		ImGui::Separator();

		if (ImGui::Button("engine->SetAudioApi()"))
			engine->SetAudioApi(selectedAudioApi);

		//ImGui::BeginCombo();
		//ImGui::PlotLines("PlitPLines", );
	}
}

const char* AudioTestWindow::GetGuiName()
{
	return u8"Audio Test";
}

ImGuiWindowFlags AudioTestWindow::GetWindowFlags()
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

			for (size_t i = 0; i < IM_ARRAYSIZE(audioFormatAndNames); i++)
			{
				if ((nativeFormats & audioFormatAndNames[i].format) != 0)
					totalContainedFlags++;
			}

			size_t flagStringsAdded = 0;
			for (size_t i = 0; i < IM_ARRAYSIZE(audioFormatAndNames); i++)
			{
				if ((nativeFormats & audioFormatAndNames[i].format) != 0)
				{
					flagStringsAdded++;
					stringStream << audioFormatAndNames[i].name;

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

	for (int i = 0; i < IM_ARRAYSIZE(deviceInfoFieldNames); i++)
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
