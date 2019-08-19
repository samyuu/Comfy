#include "IEditorComponent.h"

namespace Editor
{
	IEditorComponent::IEditorComponent(Application* parent, EditorManager* editor) : BaseWindow(parent), pvEditor(editor)
	{
	};
}