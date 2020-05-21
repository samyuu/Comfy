#pragma once
#include "Core/BaseWindow.h"
#include "IFileDropReceiver.h"
#include "Theme.h"

namespace Comfy::Studio { class Application; }

namespace Comfy::Studio::Editor
{
	class EditorManager;

	class IEditorComponent : public BaseWindow, public IFileDropReceiver
	{
	public:
		IEditorComponent(Application* parent, EditorManager* editor);
		virtual ~IEditorComponent() = default;

		virtual void Initialize() = 0;

		virtual void OnWindowBegin() {}
		virtual void OnWindowEnd() {}

	protected:
		EditorManager* pvEditor;
	};
}
