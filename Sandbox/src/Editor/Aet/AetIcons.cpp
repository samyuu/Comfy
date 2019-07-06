#include "AetIcons.h"

namespace Editor
{
	const char* GetObjTypeIcon(AetObjType type)
	{
		switch (type)
		{
		case AetObjType_Pic:
			return ICON_AETOBJPIC;
		case AetObjType_Aif:
			return ICON_AETOBJAIF;
		case AetObjType_Eff:
			return ICON_AETOBJEFF;
		case AetObjType_Nop:
		default:
			return ICON_AETOBJNOP;
		}
	}
}