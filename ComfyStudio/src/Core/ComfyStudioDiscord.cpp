#include "ComfyStudioDiscord.h"
#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION

#include "IO/Directory.h"
#include "System/Library/LibraryLoader.h"
#include <discord_game_sdk.h>

namespace Comfy::Studio
{
	namespace Discord
	{
		using DiscordCreateFunc = ::EDiscordResult(::DiscordVersion version, ::DiscordCreateParams* params, ::IDiscordCore** result);

		struct GlobalState
		{
			bool IsRuntimeEnabled = false;
			bool IsRuntimeEnabledAndInitialized = false;

			std::string_view ComfyStudioName = u8"Comfy Studio";
			::DiscordClientId ComfyStudioClientAndAppID = static_cast<::DiscordClientId>(903429580625444945);

			struct ErrorState
			{
				bool FailedToLoadDLL = false;
				bool FailedToInitializeDiscordAPI = false;
			} Error;

			struct DynamicLinkLibrary
			{
				System::LibraryLoader Loader { "discord_game_sdk.dll" };
				DiscordCreateFunc* ImportedDiscordCreateFunc = nullptr;
			} DLL = {};

			struct DiscordAPI
			{
				::IDiscordCore* Core = nullptr;
				::IDiscordUserManager* UserManager = nullptr;
				::IDiscordActivityManager* ActivityManager = nullptr;
			} API = {};

			struct ActivityData
			{
				::DiscordActivity NextFrame;
				::DiscordActivity LastFrame;
			} Activity = {};

		} Global = {};

		i64 GlobalGetCurrentUnixTime()
		{
			return static_cast<i64>(::time(nullptr));
		}

		bool GlobalGetIsRuntimeEnabled()
		{
			return Global.IsRuntimeEnabled;
		}

		void GlobalSetIsRuntimeEnabled(bool isEnabled)
		{
			if (isEnabled == Global.IsRuntimeEnabled)
				return;

			Global.IsRuntimeEnabled = isEnabled;

			Global.Error = {};

			if (Global.API.Core != nullptr)
				Global.API.Core->destroy(Global.API.Core);
			Global.API = {};

			if (Global.DLL.Loader.GetLibraryLoaded())
			{
				Global.DLL.Loader.UnLoad();
				Global.DLL.ImportedDiscordCreateFunc = nullptr;
			}

			Global.Activity = {};

			// TODO: Error logging
			if (isEnabled)
			{
				::DiscordCreateParams discordCreateParams = {}; ::DiscordCreateParamsSetDefault(&discordCreateParams);
				discordCreateParams.client_id = Global.ComfyStudioClientAndAppID;
				discordCreateParams.flags = DiscordCreateFlags_NoRequireDiscord;

				// NOTE: Always only load from exe directory so that this DLL can be deleted by the user in order to hard-disable discord integration if so desired
				const auto dllLoadDirectory = IO::Directory::GetExecutableDirectory();

				if (Global.DLL.Loader.Load(dllLoadDirectory, true))
					Global.DLL.ImportedDiscordCreateFunc = Global.DLL.Loader.GetFunctionAddress<DiscordCreateFunc>(u8"DiscordCreate");

				if (Global.DLL.ImportedDiscordCreateFunc == nullptr)
				{
					Global.Error.FailedToLoadDLL = true;
					return;
				}

				const auto discordCreateResult = Global.DLL.ImportedDiscordCreateFunc(DISCORD_VERSION, &discordCreateParams, &Global.API.Core);
				Global.API.UserManager = (Global.API.Core != nullptr) ? Global.API.Core->get_user_manager(Global.API.Core) : nullptr;
				Global.API.ActivityManager = (Global.API.Core != nullptr) ? Global.API.Core->get_activity_manager(Global.API.Core) : nullptr;

				if (Global.API.Core == nullptr || Global.API.UserManager == nullptr || Global.API.ActivityManager == nullptr)
				{
					Global.Error.FailedToInitializeDiscordAPI = true;
					return;
				}

				Global.IsRuntimeEnabledAndInitialized = true;
			}
			else
			{
				Global.IsRuntimeEnabledAndInitialized = false;
			}
		}

		bool GlobalSetStatus(const Status& inStatus)
		{
			if (!Global.IsRuntimeEnabledAndInitialized)
				return false;

			auto imageKeyEnumToString = [](ImageKey key)
			{
				switch (key)
				{
				case ImageKey::None: return u8"";
				case ImageKey::ComfyApplicatonIcon: return u8"comfy_application_icon";
				default: assert(false); return u8"";
				}
			};

			auto copyStringIntoFixedBuffer = [](char* destinationBuffer, size_t bufferSize, std::string_view stringToCopy)
			{
				if (stringToCopy.empty())
					destinationBuffer[0] = '\0';
				else
					memcpy(destinationBuffer, stringToCopy.data(), std::min<size_t>(stringToCopy.size(), bufferSize - 1));
			};

			auto& nextActivity = Global.Activity.NextFrame;
			nextActivity = {};

			nextActivity.type = DiscordActivityType_Playing;
			nextActivity.application_id = Global.ComfyStudioClientAndAppID;
			nextActivity.timestamps.start = inStatus.UnixStartTime;
			nextActivity.timestamps.end = inStatus.UnixEndTime;

			copyStringIntoFixedBuffer(nextActivity.assets.large_image, sizeof(nextActivity.assets.large_image), imageKeyEnumToString(inStatus.LargeImageKey));
			copyStringIntoFixedBuffer(nextActivity.assets.large_text, sizeof(nextActivity.assets.large_text), inStatus.LargeImageText);
			copyStringIntoFixedBuffer(nextActivity.assets.small_image, sizeof(nextActivity.assets.small_image), imageKeyEnumToString(inStatus.SmallImageKey));
			copyStringIntoFixedBuffer(nextActivity.assets.small_text, sizeof(nextActivity.assets.small_text), inStatus.SmallImageText);
			copyStringIntoFixedBuffer(nextActivity.name, sizeof(nextActivity.name), Global.ComfyStudioName);
			copyStringIntoFixedBuffer(nextActivity.state, sizeof(nextActivity.state), inStatus.State);
			copyStringIntoFixedBuffer(nextActivity.details, sizeof(nextActivity.details), inStatus.Details);

			return true;
		}

		bool GlobalOnUpdateTick()
		{
			if (!Global.IsRuntimeEnabledAndInitialized)
				return false;

			if (Global.Activity.NextFrame.application_id != 0 && memcmp(&Global.Activity.NextFrame, &Global.Activity.LastFrame, sizeof(::DiscordActivity)) != 0)
			{
				Global.Activity.LastFrame = Global.Activity.NextFrame;

				assert(Global.API.ActivityManager->update_activity != nullptr);
				Global.API.ActivityManager->update_activity(Global.API.ActivityManager, &Global.Activity.NextFrame, nullptr, [](void* callbackData, ::EDiscordResult result)
				{
					// printf("Discord update_activity callback: result = %d\n", result);
				});
			}

			assert(Global.API.Core != nullptr);
			const auto runCallbacksResult = Global.API.Core->run_callbacks(Global.API.Core);

			return (runCallbacksResult == ::DiscordResult_Ok);
		}
	}
}
#endif
