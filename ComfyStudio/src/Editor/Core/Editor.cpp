#include "Editor.h"
#include "Core/Application.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/PV/SceneRenderWindow.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	Array<ImU32, EditorColor_Count> EditorColors;

	vec4 GetColorVec4(EditorColor color)
	{
		return ImColor(GetColor(color)).Value;
	}

	ImU32 GetColor(EditorColor color)
	{
		assert(color >= 0 && color < EditorColor_Count);
		return EditorColors[color];
	}

	ImU32 GetColor(EditorColor color, float alpha)
	{
		ImVec4 colorVector = Gui::ColorConvertU32ToFloat4(GetColor(color));
		colorVector.w *= alpha;

		return Gui::ColorConvertFloat4ToU32(colorVector);
	}

	inline void SetColor(EditorColor color, ImU32 value)
	{
		EditorColors[color] = value;
	}

	void UpdateEditorColors()
	{
		SetColor(EditorColor_BaseClear, Gui::GetColorU32(ImGuiCol_TabUnfocused));
		SetColor(EditorColor_DarkClear, Gui::GetColorU32(ImGuiCol_WindowBg));
		SetColor(EditorColor_AltRow, 0xFF363636);
		SetColor(EditorColor_Grid, Gui::GetColorU32(ImGuiCol_Separator, .75f));
		SetColor(EditorColor_GridAlt, Gui::GetColorU32(ImGuiCol_Separator, .5f));
		SetColor(EditorColor_InfoColumn, Gui::GetColorU32(ImGuiCol_ScrollbarBg));
		SetColor(EditorColor_TempoMapBg, Gui::GetColorU32(ImGuiCol_MenuBarBg));
		SetColor(EditorColor_Selection, Gui::GetColorU32(ImGuiCol_TextSelectedBg));
		SetColor(EditorColor_TimelineBg, Gui::GetColorU32(ImGuiCol_ChildBg));
		SetColor(EditorColor_TimelineRowSeparator, Gui::GetColorU32(ImGuiCol_Separator));
		SetColor(EditorColor_TimelineSelection, 0x20D0D0D0);
		SetColor(EditorColor_TimelineSelectionBorder, 0x60E0E0E0);
		SetColor(EditorColor_Bar, Gui::GetColorU32(ImGuiCol_PlotLines));
		SetColor(EditorColor_Cursor, 0xFFE0E0E0);
		SetColor(EditorColor_CursorInner, GetColor(EditorColor_Cursor, 0.75f));
		SetColor(EditorColor_TextHighlight, ImColor(0.87f, 0.77f, 0.02f));
		SetColor(EditorColor_AnimatedProperty, 0xFF392A24);
		SetColor(EditorColor_KeyFrameProperty, 0xFF212132);
		SetColor(EditorColor_KeyFrame, 0xFF999999);
		SetColor(EditorColor_KeyFrameSelected, 0xFF5785D9);
		SetColor(EditorColor_KeyFrameBorder, 0xFF05070B);
	}

	EditorManager::EditorManager(Application* parent) : parent(parent)
	{
		editorComponents.reserve(3);
		AddEditorComponent<ChartEditor>();
		AddEditorComponent<AetEditor>();
		AddEditorComponent<SceneRenderWindow>();
	}

	EditorManager::~EditorManager()
	{

	}

	void EditorManager::DrawGuiMenuItems()
	{
		if (Gui::BeginMenu("Editor"))
		{
			for (const auto &component : editorComponents)
				Gui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());

			Gui::EndMenu();
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

	template<class T> void EditorManager::AddEditorComponent()
	{
		static_assert(std::is_base_of<IEditorComponent, T>::value, "T must inherit from IEditorComponent");
		editorComponents.push_back(std::move(MakeUnique<T>(parent, this)));
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
					if (Gui::Begin(component->GetGuiName(), component->GetIsGuiOpenPtr(), component->GetWindowFlags()))
						component->DrawGui();
				}
				component->OnWindowEnd();
				Gui::End();
			}
		}
	}

	void EditorManager::UpdateFileDrop()
	{
		if (parent->GetDispatchFileDrop())
		{
			const Vector<String>& droppedFiles = parent->GetDroppedFiles();

			for (const auto &component : editorComponents)
			{
				for (const String& filePath : droppedFiles)
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