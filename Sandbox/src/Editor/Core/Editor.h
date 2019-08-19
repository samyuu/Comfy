#pragma once
#include "IEditorComponent.h"
#include "Audio/DummySampleProvider.h"
#include "Audio/MemoryAudioStream.h"
#include "Audio/AudioInstance.h"
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
		void Initialize();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
		// -------------
	};
}