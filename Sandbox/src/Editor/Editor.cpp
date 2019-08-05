#include "Editor.h"
#include "Application.h"
#include "Aet/AetEditor.h"
#include "Chart/ChartEditor.h"
#include "PV/SceneRenderWindow.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	std::array<ImU32, EditorColor_Count> EditorColors;

	ImU32 GetColor(EditorColor color)
	{
		assert(color >= 0 && color < EditorColor_Count);
		return EditorColors[color];
	}

	vec4 GetColorVec4(EditorColor color)
	{
		return ImColor(GetColor(color)).Value;
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
		SetColor(EditorColor_BaseClear, ImGui::GetColorU32(ImGuiCol_TabUnfocused));
		SetColor(EditorColor_DarkClear, ImGui::GetColorU32(ImGuiCol_WindowBg));
		SetColor(EditorColor_Grid, ImGui::GetColorU32(ImGuiCol_Separator, .75f));
		SetColor(EditorColor_GridAlt, ImGui::GetColorU32(ImGuiCol_Separator, .5f));
		SetColor(EditorColor_InfoColumn, ImGui::GetColorU32(ImGuiCol_ScrollbarBg));
		SetColor(EditorColor_TempoMapBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
		SetColor(EditorColor_Selection, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
		SetColor(EditorColor_TimelineBg, ImGui::GetColorU32(ImGuiCol_ChildBg));
		SetColor(EditorColor_TimelineRowSeparator, ImGui::GetColorU32(ImGuiCol_Separator));
		SetColor(EditorColor_Bar, ImGui::GetColorU32(ImGuiCol_PlotLines));
		SetColor(EditorColor_Cursor, ImColor(0.71f, 0.54f, 0.15f));
		SetColor(EditorColor_CursorInner, GetColor(EditorColor_Cursor, 0.5f));
		SetColor(EditorColor_TextHighlight, ImColor(0.87f, 0.77f, 0.02f));
		SetColor(EditorColor_KeyFrame, GetColor(EditorColor_Cursor, 0.85f));
	}

	EditorManager::EditorManager(Application* parent) : parent(parent)
	{
		editorComponents.reserve(3);
		editorComponents.push_back(std::move(std::make_unique<ChartEditor>(parent, this)));
		editorComponents.push_back(std::move(std::make_unique<AetEditor>(parent, this)));
		editorComponents.push_back(std::move(std::make_unique<SceneRenderWindow>(parent, this)));
	}

	EditorManager::~EditorManager()
	{

	}

	void EditorManager::DrawGuiMenuItems()
	{
		if (ImGui::BeginMenu("Editor"))
		{
			for (const auto &component : editorComponents)
				ImGui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());

			ImGui::EndMenu();
		}
	}

	void EditorManager::DrawGuiWindows()
	{
		if (!initialized)
		{
			Initialize();
			initialized = true;
		}

		Update();
		DrawGui();
	}

	void EditorManager::Initialize()
	{
		for (const auto &component : editorComponents)
			component->Initialize();
	}

	void EditorManager::Update()
	{
		UpdateFileDrop();

		UpdateEditorColors();
	}

	void EditorManager::DrawGui()
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

	void EditorManager::UpdateFileDrop()
	{
		if (parent->GetDispatchFileDrop())
		{
			const std::vector<std::string>& droppedFiles = parent->GetDroppedFiles();

			for (const auto &component : editorComponents)
			{
				for (const std::string& filePath : droppedFiles)
				{
					if (component->OnFileDropped(filePath))
					{
						parent->SetFileDropDispatched();
						break;
					}
				}
			}
		}
	}
}