#pragma once
#include <FontIcons.h>
#include "FileSystem/Format/AetSet.h"

#define ICON_SETLOADER		ICON_FA_FOLDER
#define ICON_TREEVIEW		ICON_FA_LIST
#define ICON_RENDERWINDOW	ICON_FA_TH
#define ICON_INSPECTOR		ICON_FA_INFO_CIRCLE
#define ICON_PROPERTIES		ICON_FA_INFO
#define ICON_TIMELINE		ICON_FA_CLOCK

#define ICON_NAMES			ICON_FA_LIST
#define ICON_AETLAYERS		ICON_FA_LAYER_GROUP
#define ICON_AETLAYER		ICON_FA_FOLDER
#define ICON_AETLAYER_OPEN	ICON_FA_FOLDER_OPEN
#define ICON_AETREGIONS		ICON_FA_IMAGES
#define ICON_AETREGION		ICON_FA_IMAGE
#define ICON_AETREGIONNOSPR	ICON_FA_EXPAND
#define ICON_AETOBJNOP		ICON_FA_QUESTION_CIRCLE
#define ICON_AETOBJPIC		ICON_FA_CUBE
#define ICON_AETOBJAIF		ICON_FA_MUSIC
#define ICON_AETOBJEFF		ICON_FA_CUBES
#define ICON_ANIMATIONDATA	ICON_FA_INFO_CIRCLE
#define ICON_MARKERS		ICON_FA_MAP_MARKER_ALT
#define ICON_PARENT			ICON_AETOBJPIC
#define ICON_VISIBLE		ICON_FA_EYE
#define ICON_INVISIBLE		ICON_FA_EYE_SLASH
#define ICON_AUDIBLE		ICON_FA_VOLUME_UP
#define ICON_INAUDIBLE		ICON_FA_VOLUME_MUTE

#define ICON_ADD			ICON_FA_PLUS
#define ICON_MOVEUP			ICON_FA_ARROW_UP
#define ICON_MOVEDOWN		ICON_FA_ARROW_DOWN
#define ICON_DELETE			ICON_FA_TRASH

namespace Editor
{
	using namespace FileSystem;

	const char* GetObjTypeIcon(AetObjType type);
}