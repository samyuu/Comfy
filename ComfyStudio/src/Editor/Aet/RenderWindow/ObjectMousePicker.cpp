#include "ObjectMousePicker.h"
#include "Tools/TransformBox.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	ObjectMousePicker::ObjectMousePicker(const Graphics::Aet::Util::ObjCache& objectCache, const bool& windowHoveredOnMouseClick, AetItemTypePtr& selectedAetItem, AetItemTypePtr& cameraSelectedAetItem)
		: objectCache(objectCache), windowHoveredOnMouseClick(windowHoveredOnMouseClick), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem)
	{
	}

	void ObjectMousePicker::UpdateMouseInput(const vec2& mousePosition)
	{
		if (Gui::IsMouseClicked(mousePickButton))
		{
			const std::shared_ptr<Aet::Layer>* foundObject = FindObjectAtPosition(mousePosition);
			mousePickedObjectOnMouseClick = (foundObject == nullptr) ? nullptr : foundObject->get();
		}

		if (Gui::IsMouseReleased(mousePickButton) && windowHoveredOnMouseClick)
		{
			TrySelectObjectAtPosition(mousePosition);
		}
	}

	const std::shared_ptr<Aet::Layer>* ObjectMousePicker::FindObjectAtPosition(vec2 worldSpace)
	{
		const auto& selectedComp = cameraSelectedAetItem.GetCompositionRef();
		const std::shared_ptr<Aet::Layer>* foundLayer = nullptr;

		for (auto& obj : objectCache)
		{
			TransformBox box(obj.Transform, obj.Video->Size);

			if (obj.IsVisible && box.Contains(worldSpace))
			{
				for (auto& availableLayer : selectedComp->GetLayers())
				{
					if (availableLayer.get() == obj.SourceLayer || availableLayer.get() == obj.FirstParent)
						foundLayer = &availableLayer;
				}
			}
		}

		return foundLayer;
	}

	void ObjectMousePicker::TrySelectObjectAtPosition(vec2 worldSpace)
	{
		const auto* foundObject = FindObjectAtPosition(worldSpace);

		if (foundObject != nullptr && mousePickedObjectOnMouseClick == foundObject->get())
		{
			selectedAetItem.SetItem(*foundObject);
			(*foundObject)->GetParentComposition()->GuiData.TreeViewNodeOpen = true;
		}
		else
		{
			const auto& selectedComp = cameraSelectedAetItem.GetCompositionRef();
			selectedAetItem.SetItem(selectedComp);
		}
	}
}
