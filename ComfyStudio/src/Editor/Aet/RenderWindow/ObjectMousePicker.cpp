#include "ObjectMousePicker.h"
#include "Tools/TransformBox.h"
#include "ImGui/Gui.h"

namespace Editor
{
	ObjectMousePicker::ObjectMousePicker(const Vector<AetMgr::ObjCache>& objectCache, const bool& windowHoveredOnMouseClick, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem)
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

	static bool IntersectsAnyChild(const AetObj* effObj)
	{
		// TODO: Implement

		//assert(effObj->Type == AetObjType::Eff);
		//availableObj.get() == obj.AetObj
		return false;
	}

	const RefPtr<AetObj>* ObjectMousePicker::FindObjectAtPosition(vec2 worldSpace)
	{
		const auto& selectedLayer = cameraSelectedAetItem->GetAetLayerRef();
		RefPtr<AetObj>* foundObject = nullptr;

		for (auto& obj : objectCache)
		{
			TransformBox box(obj.Properties, obj.Region->GetSize());

			if (obj.Visible && box.Contains(worldSpace))
			{
				for (auto& availableObj : *selectedLayer)
				{
					if (availableObj->Type == AetObjType::Pic)
					{
						if (availableObj.get() == obj.AetObj)
						{
							foundObject = &availableObj;
						}
					}
					else if (availableObj->Type == AetObjType::Eff)
					{
						if (IntersectsAnyChild(availableObj.get()))
						{
							foundObject = &availableObj;
						}
					}
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
			(*foundObject)->GetParentLayer()->GuiData.AppendOpenNode = true;
		}
		else
		{
			const auto& selectedLayer = cameraSelectedAetItem->GetAetLayerRef();
			selectedAetItem->SetItem(selectedLayer);
		}
	}
}