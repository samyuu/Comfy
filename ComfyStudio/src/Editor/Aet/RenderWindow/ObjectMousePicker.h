#pragma once
#include "Graphics/Auth2D/AetMgr.h"
#include "Editor/Aet/AetSelection.h"

namespace Editor
{
	using namespace Graphics::Auth2D;

	class ObjectMousePicker
	{
	public:
		ObjectMousePicker(const std::vector<AetMgr::ObjCache>& objectCache, const bool& windowHoveredOnMouseClick, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem);
	
		void UpdateMouseInput(const vec2& mousePosition);

		const RefPtr<AetObj>* FindObjectAtPosition(vec2 worldSpace);
		void TrySelectObjectAtPosition(vec2 worldSpace);

	private:
		static constexpr int mousePickButton = 0;

		struct
		{
			AetItemTypePtr* selectedAetItem = nullptr;
			AetItemTypePtr* cameraSelectedAetItem = nullptr;
		};

		const bool& windowHoveredOnMouseClick;

		// NOTE: The object cache from the current frame, expected to be kept up to date by its parent
		const std::vector<AetMgr::ObjCache>& objectCache;

		// NOTE: To compare with the object on mouse release before selecting the object and prevent accidental selection.
		//		 This object is not guaranteed to stay alive and should only be used for a pointer comparison so don't try to dereference it
		const AetObj* mousePickedObjectOnMouseClick = nullptr;
	};
}