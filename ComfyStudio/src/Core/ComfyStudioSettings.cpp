#include "ComfyStudioSettings.h"
#include "IO/JSON.h"

namespace Comfy::Studio
{
	// NOTE: Underscore denote object boundaries, all IDs should follow the snake_case naming convention
	namespace AppIDs
	{
		constexpr const char* LastSessionWindowState = "last_session_window_state";
		constexpr const char* LastSessionWindowState_RestoreRegion = "restore_region";
		constexpr const char* LastSessionWindowState_Position = "position";
		constexpr const char* LastSessionWindowState_Size = "size";
		constexpr const char* LastSessionWindowState_IsFullscreen = "is_fullscreen";
		constexpr const char* LastSessionWindowState_IsMaximized = "is_maximized";
		constexpr const char* LastSessionWindowState_ActiveEditorComponent = "active_editor_component";

		constexpr const char* RecentFiles = "recent_files";
		constexpr const char* RecentFiles_ChartFiles = "chart_files";
	}

	bool ComfyStudioSettings::LoadAppData(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& appJson = loadedJson.value();

		if (const json* windowStateJson = JsonFind(appJson, AppIDs::LastSessionWindowState))
		{
			AppData.LastSessionWindowState.RestoreRegion = JsonTryGetIVec4(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegion));
			AppData.LastSessionWindowState.Position = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Position));
			AppData.LastSessionWindowState.Size = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Size));
			AppData.LastSessionWindowState.IsFullscreen = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsFullscreen));
			AppData.LastSessionWindowState.IsMaximized = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsMaximized));
			AppData.LastSessionWindowState.ActiveEditorComponent = JsonTryGetStr(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_ActiveEditorComponent));
		}

		if (const json* recentFilesJson = JsonFind(appJson, AppIDs::RecentFiles))
		{
			if (const json* chartFilesJson = JsonFind(*recentFilesJson, AppIDs::RecentFiles_ChartFiles))
			{
				std::for_each(chartFilesJson->rbegin(), chartFilesJson->rend(), [&](const json& jsonIt)
				{
					if (auto v = JsonTryGetStr(jsonIt); v.has_value())
						AppData.RecentFiles.ChartFiles.Add(v.value());
				});
			}
		}

		return true;
	}

	void ComfyStudioSettings::SaveAppData(std::string_view filePath)
	{
		json appJson = json::object();

		json& windowStateJson = appJson[AppIDs::LastSessionWindowState];
		JsonTrySetIVec4(windowStateJson[AppIDs::LastSessionWindowState_RestoreRegion], AppData.LastSessionWindowState.RestoreRegion);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Position], AppData.LastSessionWindowState.Position);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Size], AppData.LastSessionWindowState.Size);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsFullscreen], AppData.LastSessionWindowState.IsFullscreen);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsMaximized], AppData.LastSessionWindowState.IsMaximized);
		JsonTrySetStr(windowStateJson[AppIDs::LastSessionWindowState_ActiveEditorComponent], AppData.LastSessionWindowState.ActiveEditorComponent);

		json& recentFilesJson = appJson[AppIDs::RecentFiles];
		json& chartFilesJson = recentFilesJson[AppIDs::RecentFiles_ChartFiles];
		chartFilesJson = json::array();
		std::for_each(AppData.RecentFiles.ChartFiles.View().rbegin(), AppData.RecentFiles.ChartFiles.View().rend(), [&](auto& path) { chartFilesJson.emplace_back(path); });

		IO::SaveJson(filePath, appJson);
	}

	void ComfyStudioSettings::RestoreDefaultAppData()
	{
		AppData = {};
	}

	namespace UserIDs
	{
		// TODO: ...
	}

	bool ComfyStudioSettings::LoadUserData(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& userJson = loadedJson.value();
		// TODO: ...
		return true;
	}

	void ComfyStudioSettings::SaveUserData(std::string_view filePath)
	{
		json userJson = json::object();
		// TODO: ...
		IO::SaveJson(filePath, userJson);
	}

	void ComfyStudioSettings::RestoreDefaultUserData()
	{
		// TODO: ...
		UserData = {};
	}
}
