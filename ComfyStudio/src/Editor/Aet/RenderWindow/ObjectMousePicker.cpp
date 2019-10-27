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
			const RefPtr<AetObj>* foundObject = FindObjectAtPosition(mousePosition);
			mousePickedObjectOnMouseClick = (foundObject == nullptr) ? nullptr : foundObject->get();
		}

		if (Gui::IsMouseReleased(mousePickButton) && windowHoveredOnMouseClick)
		{
			TrySelectObjectAtPosition(mousePosition);
		}
	}

	const RefPtr<AetObj>* ObjectMousePicker::FindObjectAtPosition(vec2 worldSpace)
	{
		const auto& selectedLayer = cameraSelectedAetItem->GetAetLayerRef();
		RefPtr<AetObj>* foundObject = nullptr;

		for (auto& obj : objectCache)
		{
			TransformBox box(obj.Properties, obj.Region->Size);

			if (obj.Visible && box.Contains(worldSpace))
			{
				for (auto& availableObj : *selectedLayer)
				{
					if (availableObj.get() == obj.Source || availableObj.get() == obj.FirstParent)
						foundObject = &availableObj;
				}
			}
		}

		return foundObject;
	}

	void ObjectMousePicker::TrySelectObjectAtPosition(vec2 worldSpace)
	{
		const RefPtr<AetObj>* foundObject = FindObjectAtPosition(worldSpace);

		if (foundObject != nullptr && mousePickedObjectOnMouseClick == foundObject->get())
		{
			selectedAetItem->SetItem(*foundObject);
			(*foundObject)->GetParentLayer()->GuiData.TreeViewNodeOpen = true;
		}
		else
		{
			const auto& selectedLayer = cameraSelectedAetItem->GetAetLayerRef();
			selectedAetItem->SetItem(selectedLayer);
		}
	}
}