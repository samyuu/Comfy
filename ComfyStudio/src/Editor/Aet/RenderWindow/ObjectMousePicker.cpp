#include "ObjectMousePicker.h"
#include "Tools/TransformBox.h"
#include "ImGui/Gui.h"

namespace Editor
{
	using namespace Graphics;

	ObjectMousePicker::ObjectMousePicker(const std::vector<AetMgr::ObjCache>& objectCache, const bool& windowHoveredOnMouseClick, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem)
		: objectCache(objectCache), windowHoveredOnMouseClick(windowHoveredOnMouseClick), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem)
	{
	}

	void ObjectMousePicker::UpdateMouseInput(const vec2& mousePosition)
	{
		if (Gui::IsMouseClicked(mousePickButton))
		{
			const RefPtr<AetLayer>* foundObject = FindObjectAtPosition(mousePosition);
			mousePickedObjectOnMouseClick = (foundObject == nullptr) ? nullptr : foundObject->get();
		}

		if (Gui::IsMouseReleased(mousePickButton) && windowHoveredOnMouseClick)
		{
			TrySelectObjectAtPosition(mousePosition);
		}
	}

	const RefPtr<AetLayer>* ObjectMousePicker::FindObjectAtPosition(vec2 worldSpace)
	{
		const auto& selectedComp = cameraSelectedAetItem->GetAetCompositionRef();
		RefPtr<AetLayer>* foundLayer = nullptr;

		for (auto& obj : objectCache)
		{
			TransformBox box(obj.Transform, obj.Surface->Size);

			if (obj.Visible && box.Contains(worldSpace))
			{
				for (auto& availableLayer : *selectedComp)
				{
					if (availableLayer.get() == obj.Source || availableLayer.get() == obj.FirstParent)
						foundLayer = &availableLayer;
				}
			}
		}

		return foundLayer;
	}

	void ObjectMousePicker::TrySelectObjectAtPosition(vec2 worldSpace)
	{
		const RefPtr<AetLayer>* foundObject = FindObjectAtPosition(worldSpace);

		if (foundObject != nullptr && mousePickedObjectOnMouseClick == foundObject->get())
		{
			selectedAetItem->SetItem(*foundObject);
			(*foundObject)->GetParentComposition()->GuiData.TreeViewNodeOpen = true;
		}
		else
		{
			const auto& selectedComp = cameraSelectedAetItem->GetAetCompositionRef();
			selectedAetItem->SetItem(selectedComp);
		}
	}
}