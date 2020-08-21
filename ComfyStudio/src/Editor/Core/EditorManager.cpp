#include "EditorManager.h"
#include "Core/Application.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/PV/SceneEditor.h"
#include "Misc/StringUtil.h"
#include "System/ComfyData.h"

namespace Comfy::Studio::Editor
{
	namespace EditorManagerConfigIDs
	{
		constexpr std::string_view ActiveEditor = "Comfy::Studio::EditorManager::ActiveEditor";
	}

	std::array<u32, EditorColor_Count> EditorColors;

	vec4 GetColorVec4(EditorColor color)
	{
		return ImColor(GetColor(color)).Value;
	}

	u32 GetColor(EditorColor color)
	{
		assert(color >= 0 && color < EditorColor_Count);
		return EditorColors[color];
	}

	u32 GetColor(EditorColor color, float alpha)
	{
		ImVec4 colorVector = Gui::ColorConvertU32ToFloat4(GetColor(color));
		colorVector.w *= alpha;

		return Gui::ColorConvertFloat4ToU32(colorVector);
	}

	void SetColor(EditorColor color, u32 value)
	{
		EditorColors[color] = value;
	}

	void UpdateEditorColors()
	{
		using namespace Gui;
		SetColor(EditorColor_BaseClear, ImColor(0.16f, 0.16f, 0.16f, 0.0f));
		SetColor(EditorColor_DarkClear, GetColorU32(ImGuiCol_WindowBg));
		SetColor(EditorColor_AltRow, 0xFF363636);

		// TODO: These brighter highlight colors improve readability but somewhat conflict with the darker highlights of other ui elements
		SetColor(EditorColor_TreeViewSelected, ImColor(0.29f, 0.29f, 0.29f, 1.0f));
		SetColor(EditorColor_TreeViewHovered, ImColor(0.24f, 0.24f, 0.24f, 1.0f));
		SetColor(EditorColor_TreeViewActive, ImColor(0.29f, 0.29f, 0.29f, 0.8f));
		SetColor(EditorColor_TreeViewTextHighlight, ImColor(0.87f, 0.77f, 0.02f));

		// SetColor(EditorColor_Grid, GetColorU32(ImGuiCol_Separator, 0.75f));
		// SetColor(EditorColor_GridAlt, GetColorU32(ImGuiCol_Separator, 0.5f));
		SetColor(EditorColor_Grid, 0xFF373737);
		SetColor(EditorColor_GridAlt, 0xFF343434);

		SetColor(EditorColor_Waveform, 0x40616161);

		SetColor(EditorColor_Selection, GetColorU32(ImGuiCol_TextSelectedBg));
		SetColor(EditorColor_TimelineRowSeparator, 0xFF343434);

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

	EditorManager::EditorManager(Application& parent) : parent(parent)
	{
		registeredEditors.reserve(3);
		RegisterEditorComponent<ChartEditor>("Chart Editor");
		RegisterEditorComponent<AetEditor>("Aet Editor");
		RegisterEditorComponent<SceneEditor>("Scene Editor");

		const auto lastActiveName = System::Config.GetStr(EditorManagerConfigIDs::ActiveEditor).value_or("");
		const auto lastActiveIndex = FindIndexOf(registeredEditors, [&](const auto& editor) { return editor.Name == lastActiveName; });

		constexpr size_t defaultIndex = 0;
		const auto startupIndex = InBounds(lastActiveIndex, registeredEditors) ? lastActiveIndex :
			(lastActiveName == emptyEditorName) ? std::numeric_limits<size_t>::max() : defaultIndex;

		SetActiveEditor(startupIndex);
	}

	void EditorManager::GuiMenuItems()
	{
		if (Gui::BeginMenu("Workspace"))
		{
			for (size_t i = 0; i < registeredEditors.size(); i++)
			{
				bool isOpen = (i == activeEditorIndex);
				const bool isEnabled = (!isOpen);

				if (Gui::MenuItem(registeredEditors[i].Name.c_str(), nullptr, &isOpen, isEnabled))
					SetActiveEditor(i);
			}

			Gui::Separator();
			{
				bool isOpen = !InBounds(activeEditorIndex, registeredEditors);
				const bool isEnabled = (!isOpen);

				if (Gui::MenuItem(emptyEditorName.data(), nullptr, &isOpen, isEnabled))
					SetActiveEditor(std::numeric_limits<size_t>::max());
			}

			Gui::EndMenu();
		}
	}

	void EditorManager::GuiWindows()
	{
		Update();
		DrawGui();
	}

	template<typename T>
	void EditorManager::RegisterEditorComponent(std::string_view name)
	{
		static_assert(std::is_base_of_v<IEditorComponent, T>, "T must inherit from IEditorComponent");

		auto& editor = registeredEditors.emplace_back();
		editor.Name = std::string(name);
		editor.ComponentInitializer = [this]() { return std::make_unique<T>(parent, *this); };
	}

	void EditorManager::SetActiveEditor(size_t index)
	{
		activeEditorIndex = index;
		const auto* editor = IndexOrNull(activeEditorIndex, registeredEditors);

		parent.SetFormattedWindowTitle((editor != nullptr) ? editor->Name : "");
		System::Config.SetStr(EditorManagerConfigIDs::ActiveEditor, (editor != nullptr) ? editor->Name : emptyEditorName);
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
		if (!InBounds(activeEditorIndex, registeredEditors))
			return;

		auto& editor = registeredEditors[activeEditorIndex];
		if (editor.Component == nullptr)
		{
			// TODO: Remove redundant OnFirstFrame code everywhere
			editor.Component = editor.ComponentInitializer();
			editor.Component->OnFirstFrame();
		}

		editor.Component->OnWindowBegin();
		{
			if (Gui::Begin(editor.Component->GetName(), nullptr, editor.Component->GetFlags()))
				editor.Component->Gui();
		}
		editor.Component->OnWindowEnd();
		Gui::End();
	}

	void EditorManager::UpdateFileDrop()
	{
		if (!parent.GetHost().GetDispatchFileDrop())
			return;

		const auto& droppedFiles = parent.GetHost().GetDroppedFiles();

		for (const auto& component : registeredEditors)
		{
			for (const auto& filePath : droppedFiles)
			{
				if (component.Component->OnFileDropped(filePath))
				{
					parent.GetHost().SetFileDropDispatched();
					break;
				}
			}
		}
	}
}
