#pragma once
#include "../BaseWindow.h"

class Application;

namespace Editor
{
	class PvEditor;

	class IEditorComponent : public BaseWindow
	{
	public:
		IEditorComponent(Application* parent, PvEditor* editor);
		virtual void Initialize() = 0;

		virtual void OnResumePlayback() {};
		virtual void OnPausePlayback() {};

	protected:
		PvEditor* pvEditor;
	};
}