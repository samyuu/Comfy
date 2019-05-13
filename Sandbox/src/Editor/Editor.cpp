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


		// Audio API Test
		// --------------
		if (false)
		{
			if (parent->HasFocusBeenLost())
				AudioEngine::GetInstance()->SetAudioApi(AUDIO_API_WASAPI);
			else if (parent->HasFocusBeenGained())
				AudioEngine::GetInstance()->SetAudioApi(AUDIO_API_ASIO);
		}
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
		playbackTime += ImGui::GetIO().DeltaTime;

		if (songInstance->GetIsPlaying() && songStream != nullptr)
			playbackTime = songInstance->GetPosition();
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
		auto newSongStream = std::make_shared<MemoryAudioStream>(filePath);

		songInstance->SetSampleProvider(newSongStream.get());
		songInstance->SetPosition(playbackTime);
		songDuration = songInstance->GetDuration();

		if (songStream != nullptr)
			songStream->Dispose();

		// adds some copy overhead but we don't want to delete the old pointer while the new one is still loading
		songStream = newSongStream;

		return true;
	}

	void PvEditor::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = playbackTime;

		isPlaying = true;
		songInstance->SetIsPlaying(true);
		songInstance->SetPosition(playbackTime);

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
		playbackTime = playbackTimeOnPlaybackStart;
		PausePlayback();
	}
}