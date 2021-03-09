#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Common/RecentFilesList.h"

namespace Comfy::Studio
{
	struct ComfyStudioSettings
	{
	public:
		static constexpr std::string_view AppFilePath = "settings_app.json";
		static constexpr std::string_view UserFilePath = "settings_user.json";

	public:
		bool LoadAppData(std::string_view filePath = AppFilePath);
		void SaveAppData(std::string_view filePath = AppFilePath);
		void RestoreDefaultAppData();

		bool LoadUserData(std::string_view filePath = UserFilePath);
		void SaveUserData(std::string_view filePath = UserFilePath);
		void RestoreDefaultUserData();

	public:
		// NOTE: Loaded at startup and saved on exit
		struct AppSettingsData
		{
			struct
			{
				std::optional<ivec4> RestoreRegion;
				std::optional<ivec2> Position;
				std::optional<ivec2> Size;
				std::optional<bool> IsFullscreen;
				std::optional<bool> IsMaximized;
				std::optional<std::string> ActiveEditorComponent;
			} LastSessionWindowState;

			struct
			{
				Editor::RecentFilesList ChartFiles;
			} RecentFiles;
		} AppData;

		// NOTE: Loaded at startup but only saved when manually edited by the user via a settings window
		struct UserSettingsData
		{
			// TODO: ...
		} UserData;
	};

	inline ComfyStudioSettings GlobalSettings = {};
}
