#include "Editor.h"
#include "Core/Application.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Chart/ChartEditor.h"
//#include "Editor/PV/SceneRenderWindow.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	std::array<ImU32, EditorColor_Count> EditorColors;

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
		using namespace Gui;
		SetColor(EditorColor_BaseClear, GetColorU32(ImGuiCol_TabUnfocused));
		SetColor(EditorColor_DarkClear, GetColorU32(ImGuiCol_WindowBg));
		SetColor(EditorColor_AltRow, 0xFF363636);

		// TODO: These brighter highlight colors improve readability but somewhat conflict with the darker highlights of other ui elements
		SetColor(EditorColor_TreeViewSelected, ImColor(0.29f, 0.29f, 0.29f, 1.0f));
		SetColor(EditorColor_TreeViewHovered, ImColor(0.24f, 0.24f, 0.24f, 1.0f));
		SetColor(EditorColor_TreeViewActive, ImColor(0.29f, 0.29f, 0.29f, 0.8f));
		SetColor(EditorColor_TreeViewTextHighlight, ImColor(0.87f, 0.77f, 0.02f));

		SetColor(EditorColor_Grid, GetColorU32(ImGuiCol_Separator, 0.75f));
		SetColor(EditorColor_GridAlt, GetColorU32(ImGuiCol_Separator, 0.5f));
		SetColor(EditorColor_Selection, GetColorU32(ImGuiCol_TextSelectedBg));
		SetColor(EditorColor_TimelineRowSeparator, GetColorU32(ImGuiCol_Separator));
		SetColor(EditorColor_TimelineSelection, 0x20D0D0D0);
		SetColor(EditorColor_TimelineSelectionBorder, 0x60E0E0E0);
		SetColor(EditorColor_Bar, GetColorU32(ImGuiCol_PlotLines));
		SetColor(EditorColor_Cursor, 0xFFE0E0E0);
		SetColor(EditorColor_CursorInner, GetColor(EditorColor_Cursor, 0.75f));
		SetColor(EditorColor_AnimatedProperty, 0xFF392A24);
		SetColor(EditorColor_KeyFrameProperty, 0xFF212132);
		SetColor(EditorColor_KeyFrame, 0xFFBCBCBC);
		SetColor(EditorColor_KeyFrameConnection, 0xFF626262);
		SetColor(EditorColor_KeyFrameConnectionAlt, 0xFF4E4E4E);
		SetColor(EditorColor_KeyFrameSelected, 0xFF5785D9);
		SetColor(EditorColor_KeyFrameBorder, 0xFF1A1B1B);
	}

	EditorManager::EditorManager(Application* parent) : parent(parent)
	{
		editorComponents.reserve(3);
		AddEditorComponent<ChartEditor>(false);
		AddEditorComponent<AetEditor>(true);
		//AddEditorComponent<SceneRenderWindow>(false);
	}

	EditorManager::~EditorManager()
	{

	}

	void EditorManager::DrawGuiMenuItems()
	{
		if (Gui::BeginMenu("Editor"))
		{
			for (auto& component : editorComponents)
				Gui::MenuItem(component.Component->GetGuiName(), nullptr, component.Component->GetIsGuiOpenPtr());

			Gui::EndMenu();
		}
	}

	void EditorManager::DrawGuiWindows()
	{
		if (!hasBeenInitialized)
		{
			Initialize();
			hasBeenInitialized = true;
		}

		Update();
		DrawGui();
	}

	template<class T>
	void EditorManager::AddEditorComponent(bool opened)
	{
		static_assert(std::is_base_of<IEditorComponent, T>::value, "T must inherit from IEditorComponent");
		editorComponents.push_back({ false, std::move(MakeUnique<T>(parent, this)) });
	
		if (!opened)
			editorComponents.back().Component->CloseWindow();
	}

	void EditorManager::Initialize()
	{
		return;
	}

	void EditorManager::Update()
	{
		UpdateFileDrop();

#if defined(COMFY_DEBUG)
		// NOTE: Only support live updating during debug builds
		UpdateEditorColors();
#else
		// NOTE: In case the theme is being changed dynamically on startup
		constexpr int frameCountThreshold = 3;

		if (Gui::GetFrameCount() <= frameCountThreshold)
			UpdateEditorColors();
#endif
	}

	void EditorManager::DrawGui()
	{
		for (auto& component : editorComponents)
		{
			if (*component.Component->GetIsGuiOpenPtr())
			{
				if (!component.HasBeenInitialized)
				{
					component.Component->Initialize();
					component.HasBeenInitialized = true;
				}

				component.Component->OnWindowBegin();
				{
					if (Gui::Begin(component.Component->GetGuiName(), component.Component->GetIsGuiOpenPtr(), component.Component->GetWindowFlags()))
						component.Component->DrawGui();
				}
				component.Component->OnWindowEnd();
				Gui::End();
			}
		}
	}

	void EditorManager::UpdateFileDrop()
	{
		if (parent->GetHost().GetDispatchFileDrop())
		{
			const auto& droppedFiles = parent->GetHost().GetDroppedFiles();

			for (const auto &component : editorComponents)
			{
				for (const std::string& filePath : droppedFiles)
				{
					if (component.Component->OnFileDropped(filePath))
					{
						parent->GetHost().SetFileDropDispatched();
						break;
					}
				}
			}
		}
	}
}