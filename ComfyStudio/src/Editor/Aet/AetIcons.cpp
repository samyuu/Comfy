#include "AetIcons.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	const char* GetItemTypeIcon(Aet::ItemType type)
	{
		switch (type)
		{
		case Aet::ItemType::Video:
			return ICON_AETITEMVIDEO;
		case Aet::ItemType::Audio:
			return ICON_AETITEMAUDIO;
		case Aet::ItemType::Composition:
			return ICON_AETITEMCOMP;
		case Aet::ItemType::None:
		default:
			return ICON_AETITEMNONE;
		}
	}
}
