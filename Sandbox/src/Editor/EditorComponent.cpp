#include "IEditorComponent.h"

namespace Editor
{
	IEditorComponent::IEditorComponent(Application* parent, PvEditor* editor) : BaseWindow(parent), pvEditor(editor)
	{
	};
}