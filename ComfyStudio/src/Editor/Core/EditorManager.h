#pragma once
#include "CoreTypes.h"
#include "IEditorComponent.h"

namespace Comfy::Studio { class Application; }

namespace Comfy::Studio::Editor
{
	class EditorManager
	{
	public:
		// NOTE: Constructors / Destructors:
		EditorManager(Application* parent);
		~EditorManager();

		// NOTE: Application methods:
		void DrawGuiMenuItems();
		void DrawGuiWindows();

	private:
		// NOTE: Base members
		Application* parent = nullptr;

		struct ComponentEntry
		{
			bool HasBeenInitialized;
			std::unique_ptr<IEditorComponent> Component;
		};

		std::vector<ComponentEntry> editorComponents;
		bool hasBeenInitialized = false;

	private:
		// NOTE: Base methods
		template <typename T> 
		void AddEditorComponent(bool opened);

		void Initialize();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
	};
}
