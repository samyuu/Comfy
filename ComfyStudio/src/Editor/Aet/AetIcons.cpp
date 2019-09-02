#include "AetIcons.h"

namespace Editor
{
	const char* GetObjTypeIcon(AetObjType type)
	{
		switch (type)
		{
		case AetObjType::Pic:
			return ICON_AETOBJPIC;
		case AetObjType::Aif:
			return ICON_AETOBJAIF;
		case AetObjType::Eff:
			return ICON_AETOBJEFF;
		case AetObjType::Nop:
		default:
			return ICON_AETOBJNOP;
		}
	}
}