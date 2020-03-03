#include "IEditorComponent.h"

namespace Comfy::Editor
{
	IEditorComponent::IEditorComponent(Application* parent, EditorManager* editor) : BaseWindow(parent), pvEditor(editor)
	{
	};
}
