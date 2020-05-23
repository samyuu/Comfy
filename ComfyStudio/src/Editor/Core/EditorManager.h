#pragma once
#include "CoreTypes.h"
#include "IEditorComponent.h"

namespace Comfy::Studio::Editor
{
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

		struct ComponentEntry
		{
			bool IsFirstFrame;
			std::unique_ptr<IEditorComponent> Component;
		};

		std::vector<ComponentEntry> editorComponents;
		bool isFirstFrame = false;

	private:
		template <typename T> 
		void AddEditorComponent(bool opened);

		void OnFirstFrame();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
	};
}
