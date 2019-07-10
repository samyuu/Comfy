#pragma once
#include <FontIcons.h>
#include "FileSystem/Format/AetSet.h"

#define ICON_NAMES			ICON_FA_LIST
#define ICON_AETLAYERS		ICON_FA_LAYER_GROUP
#define ICON_AETLAYER		ICON_FA_FOLDER
#define ICON_AETLAYER_OPEN	ICON_FA_FOLDER_OPEN
#define ICON_AETREGIONS		ICON_FA_IMAGES
#define ICON_AETREGION		ICON_FA_IMAGE
#define ICON_AETOBJNOP		ICON_FA_QUESTION_CIRCLE
#define ICON_AETOBJPIC		ICON_FA_CUBE
#define ICON_AETOBJAIF		ICON_FA_VOLUME_UP
#define ICON_AETOBJEFF		ICON_FA_CUBES
#define ICON_ANIMATIONDATA	ICON_FA_INFO_CIRCLE
#define ICON_PARENT			ICON_FA_INFO_CIRCLE
#define ICON_VISIBLE		ICON_FA_EYE
#define ICON_INVISIBLE		ICON_FA_EYE_SLASH

namespace Editor
{
	using namespace FileSystem;

	const char* GetObjTypeIcon(AetObjType type);
}