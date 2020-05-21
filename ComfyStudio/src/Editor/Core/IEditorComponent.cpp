#include "IEditorComponent.h"

namespace Comfy::Studio::Editor
{
	IEditorComponent::IEditorComponent(Application* parent, EditorManager* editor) : BaseWindow(parent), pvEditor(editor)
	{
	};
}
