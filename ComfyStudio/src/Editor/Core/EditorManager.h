#pragma once
#include "IEditorComponent.h"
#include "Core/CoreTypes.h"

class Application;

namespace Editor
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
			UniquePtr<IEditorComponent> Component;
		};

		std::vector<ComponentEntry> editorComponents;
		bool hasBeenInitialized = false;

	private:
		// NOTE: Base methods
		template <class T> 
		void AddEditorComponent(bool opened);

		void Initialize();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
	};
}
