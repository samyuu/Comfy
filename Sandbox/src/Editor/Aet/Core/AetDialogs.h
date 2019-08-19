#pragma once
#include "Types.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AddAetObjDialog
	{
	public:
		bool DrawGui(Aet* aet, AetLayer* layer);

		bool* GetIsGuiOpenPtr();
		const char* GetGuiName();

	private:
		char newObjNameBuffer[255];

		int32_t newTypeIndex = static_cast<int>(AetObjType::Pic);
		int32_t newRegionIndex = -1;

		bool isGuiOpen = true;
	};
}