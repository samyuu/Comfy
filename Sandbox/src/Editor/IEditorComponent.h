#pragma once
#include "../BaseWindow.h"
#include "Theme.h"

class Application;

namespace Editor
{
	class PvEditor;

	class IEditorComponent : public BaseWindow
	{
	public:
		IEditorComponent(Application* parent, PvEditor* editor);
		virtual void Initialize() = 0;

		virtual void OnPlaybackResumed() {};
		virtual void OnPlaybackPaused() {};
		virtual void OnPlaybackStopped() {};

		virtual void OnLoad() {};

	protected:
		PvEditor* pvEditor;
	};
}