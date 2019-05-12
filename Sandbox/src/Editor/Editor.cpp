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

	void PvEditor::Initialize()
	{
		for (const auto &component : editorComponents)
			component->Initialize();
	}

	void PvEditor::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = playbackTime;

		isPlaying = true;
		songInstance->SetIsPlaying(true);
		songInstance->SetPosition(playbackTime);
	
		for (const auto &component : editorComponents)
			component->OnResumePlayback();
	}

	void PvEditor::PausePlayback()
	{
		songInstance->SetIsPlaying(false);
		isPlaying = false;

		for (const auto &component : editorComponents)
			component->OnPausePlayback();
	}

	void PvEditor::StopPlayback()
	{
		playbackTime = playbackTimeOnPlaybackStart;
		PausePlayback();
	}
}