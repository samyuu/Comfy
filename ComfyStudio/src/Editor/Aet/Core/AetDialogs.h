#pragma once
#include "Types.h"
#include "Graphics/Auth2D/AetSet.h"

namespace Editor
{
	class AddAetObjDialog
	{
	public:
		bool DrawGui(Graphics::Aet* aet, Graphics::AetLayer* layer);

		bool* GetIsGuiOpenPtr();
		const char* GetGuiName();

	private:
		char newObjNameBuffer[255];

		int32_t newTypeIndex = static_cast<int>(Graphics::AetObjType::Pic);
		int32_t newRegionIndex = -1;

		bool isGuiOpen = true;
	};
}