#pragma once
#include "Types.h"

namespace Comfy::IO
{
	namespace Shell
	{
		constexpr std::string_view FileLinkExtension = ".lnk";

		COMFY_NODISCARD bool IsFileLink(std::string filePath);
		COMFY_NODISCARD std::string ResolveFileLink(std::string_view lnkFilePath);

		void OpenInExplorer(std::string_view filePath);

		void OpenExplorerProperties(std::string_view filePath);

		void OpenWithDefaultProgram(std::string_view filePath);

		namespace Custom
		{
			constexpr uint32_t ItemBaseID = 0x666;

			enum class ItemType
			{
				VisualGroupStart,
				VisualGroupEnd,
				Checkbox,
			};

			struct Item
			{
				ItemType Type;
				std::string_view Label;
				union DataUnion
				{
					bool* CheckboxChecked;
				} Data;
			};
		}

		class FileDialog
		{
		public:
			static constexpr std::string_view AllFilesFilterName = "All Files";
			static constexpr std::string_view AllFilesFilterSpec = "*.*";

		public:
			struct FileFilter
			{
				// NOTE: In the format "File Type Name" without spec prefix
				std::string Name;
				// NOTE: In the format "*.ext" for a single format and "*.ext;*.ext;*.ext" for a list
				std::string Spec;
			};

			std::string_view Title;
			std::string FileName;
			std::string DefaultExtension;
			std::vector<FileFilter> Filters;
			u32 FilterIndex = 0;
			void* ParentWindowHandle = nullptr;
			std::vector<Custom::Item> CustomizeItems;
			std::string OutFilePath;

		public:
			COMFY_NODISCARD bool OpenRead();
			COMFY_NODISCARD bool OpenSave();
			COMFY_NODISCARD bool OpenSelectFolder();

		private:
			bool InternalCreateAndShowDialog(bool openSave, bool pickFolder);
		};
	}
}
