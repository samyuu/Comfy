#pragma once
#include "IEditorComponent.h"
#include <vector>
#include <string>

class Application;

namespace Editor
{
	class EditorManager
	{
	public:
		// Constructors / Destructors:
		// ---------------------------
		EditorManager(Application* parent);
		~EditorManager();
		// ---------------------------

		// Application Methods:
		// --------------------
		void DrawGuiMenuItems();
		void DrawGuiWindows();
		// --------------------

	private:
		// Base Members:
		// -------------
		Application* parent;

		std::vector<UniquePtr<IEditorComponent>> editorComponents;
		bool initialized = false;
		// -------------

		// Base Methods:
		// -------------
		template <class T> void AddEditorComponent();

		void Initialize();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
		// -------------
	};
}