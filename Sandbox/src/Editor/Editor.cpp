#include <boost/algorithm/string/predicate.hpp>
#include "Editor.h"
#include "Chart/TargetTimeline.h"
#include "../Application.h"

namespace Editor
{
	PvEditor::PvEditor(Application* parent) : parent(parent)
	{
		editorComponents.reserve(1);
		editorComponents.push_back(std::move(std::make_unique<TargetTimeline>(parent, this)));
	}

	PvEditor::~PvEditor()
	{

	}

	void PvEditor::DrawGuiMenuItems()
	{
		if (ImGui::BeginMenu("Editor"))
		{
			for (const auto &component : editorComponents)
				ImGui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());

			ImGui::EndMenu();
		}
	}

	void PvEditor::DrawGuiWindows()
	{
		if (!initialized)
		{
			Initialize();
			initialized = true;
		}

		Update();
		DrawGui();
	}

	void PvEditor::Initialize()
	{
		auto audioEngine = AudioEngine::GetInstance();
		audioEngine->OpenStream();
		audioEngine->StartStream();

		songInstance = std::make_shared<AudioInstance>(&dummySampleProvider, "PvEditor::SongInstance");
		songInstance->SetPlayPastEnd(true);
		audioEngine->AddAudioInstance(songInstance);

		for (const auto &component : editorComponents)
			component->Initialize();
	}

	void PvEditor::Update()
	{
		if (!isPlaying)
		{
			UpdateFileDrop();
		}

		if (isPlaying)
		{
			UpdatePlayback();
		}

		// Update Colors:
		// --------------
		{
			editorColors[EditorColor_Grid] = ImGui::GetColorU32(ImGuiCol_Separator, .75f);
			editorColors[EditorColor_GridAlt] = ImGui::GetColorU32(ImGuiCol_Separator, .5f);
			editorColors[EditorColor_InfoColumn] = ImGui::GetColorU32(ImGuiCol_ScrollbarBg);
			editorColors[EditorColor_TempoMapBg] = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
			editorColors[EditorColor_Selection] = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
			editorColors[EditorColor_TimelineBg] = ImGui::GetColorU32(ImGuiCol_DockingEmptyBg);
			editorColors[EditorColor_TimelineRowSeparator] = ImGui::GetColorU32(ImGuiCol_Separator);
			editorColors[EditorColor_Bar] = ImGui::GetColorU32(ImGuiCol_PlotLines);
			editorColors[EditorColor_Cursor] = ImColor(0.71f, 0.54f, 0.15f);
		}
		// --------------
	}

	void PvEditor::DrawGui()
	{
		for (const auto &component : editorComponents)
		{
			if (*component->GetIsGuiOpenPtr())
			{
				if (ImGui::Begin(component->GetGuiName(), component->GetIsGuiOpenPtr(), component->GetWindowFlags()))
					component->DrawGui();

				ImGui::End();
			}
		}
	}

	void PvEditor::UpdateFileDrop()
	{
		if (!parent->GetDispatchFileDrop())
			return;

		auto droppedFiles = parent->GetDroppedFiles();
		for (size_t i = 0; i < droppedFiles->size(); i++)
		{
			if (Load(droppedFiles->at(i)))
				parent->SetFileDropDispatched();
		}
	}

	void PvEditor::UpdatePlayback()
	{
		//playbackTime += ImGui::GetIO().DeltaTime;

		//if (songInstance->GetIsPlaying())
		//	playbackTime = songInstance->GetPosition() - songStartOffset;
	}

	bool PvEditor::Load(const std::string& filePath)
	{
		bool loaded = false;

		for (size_t e = 0; e < IM_ARRAYSIZE(audioFileExtensions); e++)
		{
			if (boost::iends_with(filePath, audioFileExtensions[e]))
			{
				if (LoadSong(filePath))
				{
					loaded = true;
					break;
				}
			}
		}

		for (const auto &component : editorComponents)
			component->OnLoad();

		return loaded;
	}

	bool PvEditor::LoadSong(const std::string& filePath)
	{
		TimeSpan playbackTime = GetPlaybackTime();
		auto newSongStream = std::make_shared<MemoryAudioStream>(filePath);

		songInstance->SetSampleProvider(newSongStream.get());
		songDuration = songInstance->GetDuration();

		if (songStream != nullptr)
			songStream->Dispose();

		// adds some copy overhead but we don't want to delete the old pointer while the new one is still loading
		songStream = newSongStream;

		SetPlaybackTime(playbackTime);
		return true;
	}

	TimeSpan PvEditor::GetPlaybackTime()
	{
		return songInstance->GetPosition() - songStartOffset;
	}

	void PvEditor::SetPlaybackTime(TimeSpan value)
	{
		songInstance->SetPosition(value + songStartOffset);
	}

	bool PvEditor::GetIsPlayback()
	{
		return isPlaying;
	}

	void PvEditor::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = GetPlaybackTime();

		isPlaying = true;
		songInstance->SetIsPlaying(true);

		for (const auto &component : editorComponents)
			component->OnPlaybackResumed();
	}

	void PvEditor::PausePlayback()
	{
		songInstance->SetIsPlaying(false);
		isPlaying = false;

		for (const auto &component : editorComponents)
			component->OnPlaybackPaused();
	}

	void PvEditor::StopPlayback()
	{
		SetPlaybackTime(playbackTimeOnPlaybackStart);
		PausePlayback();

		for (const auto &component : editorComponents)
			component->OnPlaybackStopped();
	}
}