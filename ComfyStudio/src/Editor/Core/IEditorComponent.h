#pragma once
#include "Window/ApplicationHost.h"
#include "Core/BaseWindow.h"
#include "IFileDropReceiver.h"
#include "Theme.h"

namespace Comfy::Studio::Editor
{
	class EditorManager;

	class IEditorComponent : public BaseWindow, public IFileDropReceiver
	{
	public:
		IEditorComponent(ComfyStudioApplication& parentApplication, EditorManager& parentEditor) : BaseWindow(parentApplication), parentEditor(parentEditor) {}
		virtual ~IEditorComponent() = default;

	public:
		virtual void OnWindowBegin() {}
		virtual void OnWindowEnd() {}

		virtual void GuiMenu() {}
		virtual void OnEditorComponentMadeActive() {}
		virtual ApplicationHostCloseResponse OnApplicationClosing() { return ApplicationHostCloseResponse::Exit; }

		virtual void OnExclusiveGui() {}

	protected:
		EditorManager& parentEditor;
	};
}
