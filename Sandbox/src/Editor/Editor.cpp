#include "Editor.h"
#include "Aet/AetEditor.h"
#include "Chart/TargetTimeline.h"
#include "PV/SceneRenderWindow.h"
#include "Application.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	std::array<ImU32, EditorColor_Max> EditorColors;

	ImU32 GetColor(EditorColor color)
	{
		return DEBUG_RELEASE(EditorColors.at(color), EditorColors[color]);
	}

	vec4 GetColorVec4(EditorColor color)
	{
		ImVec4 imVec4 = ImColor(GetColor(color)).Value;
		return vec4(imVec4.x, imVec4.y, imVec4.z, imVec4.w);
	}

	ImU32 GetColor(EditorColor color, float alpha)
	{
		ImVec4 colorVector = ImGui::ColorConvertU32ToFloat4(GetColor(color));
		colorVector.w *= alpha;

		return ImGui::ColorConvertFloat4ToU32(colorVector);
	}

	inline void SetColor(EditorColor color, ImU32 value)
	{
		EditorColors[color] = value;
	}

	void UpdateEditorColors()
	{
		//SetColor(EditorColor_BaseClear, ImGui::GetColorU32(ImVec4(.12f, .12f, .12f, 1.0f)));
		SetColor(EditorColor_BaseClear, ImGui::GetColorU32(ImGuiCol_TabUnfocused));
		SetColor(EditorColor_DarkClear, ImGui::GetColorU32(ImVec4(.10f, .10f, .10f, 1.0f)));
		SetColor(EditorColor_Grid, ImGui::GetColorU32(ImGuiCol_Separator, .75f));
		SetColor(EditorColor_GridAlt, ImGui::GetColorU32(ImGuiCol_Separator, .5f));
		SetColor(EditorColor_InfoColumn, ImGui::GetColorU32(ImGuiCol_ScrollbarBg));
		SetColor(EditorColor_TempoMapBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
		SetColor(EditorColor_Selection, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
		SetColor(EditorColor_TimelineBg, ImGui::GetColorU32(ImGuiCol_DockingEmptyBg));
		SetColor(EditorColor_TimelineRowSeparator, ImGui::GetColorU32(ImGuiCol_Separator));
		SetColor(EditorColor_Bar, ImGui::GetColorU32(ImGuiCol_PlotLines));
		SetColor(EditorColor_Cursor, ImColor(0.71f, 0.54f, 0.15f));
		SetColor(EditorColor_CursorInner, GetColor(EditorColor_Cursor, 0.5f));
		SetColor(EditorColor_TextHighlight, ImColor(0.87f, 0.77f, 0.02f));
		SetColor(EditorColor_KeyFrame, GetColor(EditorColor_Cursor, 0.85f));
	}

	PvEditor::PvEditor(Application* parent) : parent(parent)
	{
		editorComponents.reserve(3);
		editorComponents.push_back(std::move(std::make_unique<TargetTimeline>(parent, this)));
		editorComponents.push_back(std::move(std::make_unique<AetEditor>(parent, this)));
		editorComponents.push_back(std::move(std::make_unique<SceneRenderWindow>(parent, this)));
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

		UpdateEditorColors();
	}

	void PvEditor::DrawGui()
	{
		for (const auto &component : editorComponents)
		{
			if (*component->GetIsGuiOpenPtr())
			{
				component->OnWindowBegin();
				{
					if (ImGui::Begin(component->GetGuiName(), component->GetIsGuiOpenPtr(), component->GetWindowFlags()))
						component->DrawGui();
				}
				component->OnWindowEnd();
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

	bool PvEditor::Load(const std::string& filePath)
	{
		bool loaded = false;

		for (size_t e = 0; e < audioFileExtensions.size(); e++)
		{
			if (EndsWithInsensitive(filePath, audioFileExtensions[e]))
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

	TimeSpan PvEditor::GetPlaybackTime() const
	{
		return songInstance->GetPosition() - songStartOffset;
	}

	void PvEditor::SetPlaybackTime(TimeSpan value)
	{
		songInstance->SetPosition(value + songStartOffset);
	}

	bool PvEditor::GetIsPlayback() const
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