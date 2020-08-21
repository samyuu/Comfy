#pragma once
#include "CoreTypes.h"
#include "IEditorComponent.h"
#include <functional>

namespace Comfy::Studio::Editor
{
	// TODO: Impl (?), merge with Application and rename to ComfyStudioApp (?)
	class EditorManager : NonCopyable
	{
	public:
		EditorManager(Application& parent);
		~EditorManager() = default;

	public:
		void GuiMenuItems();
		void GuiWindows();

	private:
		Application& parent;

		struct EditorComponent
		{
			std::string Name;
			std::unique_ptr<IEditorComponent> Component;
			std::function<std::unique_ptr<IEditorComponent>()> ComponentInitializer;
		};

		std::vector<EditorComponent> registeredEditors;
		size_t activeEditorIndex = std::numeric_limits<size_t>::max();

	private:
		template <typename T>
		void RegisterEditorComponent(std::string_view name);

		void SetActiveEditor(size_t index);

		void Update();
		void DrawGui();
		void UpdateFileDrop();
	};
}
