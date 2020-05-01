#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"

namespace Comfy::IO
{
	namespace Shell
	{
		const std::vector<std::string> AllFilesFilter = { "All Files (*.*)", "*.*" };

		COMFY_NODISCARD std::string ResolveFileLink(std::string_view lnkFilePath);

		void OpenInExplorer(std::string_view filePath);
		void OpenExplorerProperties(std::string_view filePath);

		void OpenWithDefaultProgram(std::string_view filePath);

		bool CreateOpenFileDialog(std::string& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);
		bool CreateSaveFileDialog(std::string& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);
	}
}
