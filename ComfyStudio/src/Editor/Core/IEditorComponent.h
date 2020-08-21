#pragma once
#include "Core/BaseWindow.h"
#include "IFileDropReceiver.h"
#include "Theme.h"

namespace Comfy::Studio::Editor
{
	class EditorManager;

	class IEditorComponent : public BaseWindow, public IFileDropReceiver
	{
	public:
		IEditorComponent(Application& parentApplication, EditorManager& parentEditor) : BaseWindow(parentApplication), parentEditor(parentEditor) {}
		virtual ~IEditorComponent() = default;

	public:
		virtual void OnWindowBegin() {}
		virtual void OnWindowEnd() {}

	protected:
		EditorManager& parentEditor;
	};
}
