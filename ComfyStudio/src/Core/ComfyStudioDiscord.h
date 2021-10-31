#pragma once
#include "Types.h"

// NOTE: If this is disabled then the discord headers won't even be touched.
//		 If it *is* enabled but discord isn't available or anything else fails
//		 then all of these interface functions will simply be null ops
#define COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION 1

namespace Comfy::Studio
{
	// NOTE: All of this is global because.. there is no point in doing anything else.
	//		 Expected to be called from the main render thread
	namespace Discord
	{
#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		enum class ImageKey : i32
		{
			None,
			ComfyApplicatonIcon,
			// TODO: Difficulty icons to use as SmallImageKey (?)
			Count
		};

		struct Status
		{
			ImageKey LargeImageKey;
			ImageKey SmallImageKey;
			std::string LargeImageText;
			std::string SmallImageText;

			std::string State;
			std::string Details;

			i64 UnixStartTime;
			i64 UnixEndTime;
		};

		extern i64 GlobalGetCurrentUnixTime();

		// NOTE: The #define can be used to disable integration at compile time and these function to disable at runtime.
		//		 *Disabled* is the default state. DLLs are loaded and initialized once enabled.
		extern bool GlobalGetIsRuntimeEnabled();
		extern void GlobalSetIsRuntimeEnabled(bool isEnabled);

		// NOTE: Can be called once or even multiple times per frame without any problem, requests are only send if a changed is detected
		extern bool GlobalSetStatus(const Status& inStatus);

		// NOTE: Call once at either start or end of each frame
		extern bool GlobalOnUpdateTick();
#else
		struct Status { u8 Dummy; };

		inline i64 GlobalGetCurrentUnixTime() { return 0; }

		inline bool GlobalGetIsRuntimeEnabled() { return false; }
		inline void GlobalSetIsRuntimeEnabled(bool) {}

		inline bool GlobalSetStatus(const Status&) { return false; }
		inline bool GlobalOnUpdateTick() { return false; }
#endif
	}
}
