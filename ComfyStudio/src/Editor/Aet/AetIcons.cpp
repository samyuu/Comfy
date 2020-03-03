#include "AetIcons.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	const char* GetLayerTypeIcon(AetLayerType type)
	{
		switch (type)
		{
		case AetLayerType::Pic:
			return ICON_AETLAYERPIC;
		case AetLayerType::Aif:
			return ICON_AETLAYERAIF;
		case AetLayerType::Eff:
			return ICON_AETLAYEREFF;
		case AetLayerType::Nop:
		default:
			return ICON_AETLAYERNOP;
		}
	}
}
