#include "LicenseWindow.h"
#include "ImGui/Gui.h"
#include "IO/File.h"
#include "Misc/StringParseHelper.h"
#include "Core/ComfyData.h"

namespace Comfy::Studio
{
	bool LicenseWindow::DrawGui()
	{
		if (!dataLoaded)
		{
			LoadLicenseData();
			dataLoaded = true;
		}

		constexpr ImGuiWindowFlags scrollBarWindowFlags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

		Gui::BulletText("License:");
		Gui::Separator();

		Gui::BeginChild("ListChild##LicenseWindow", vec2(Gui::GetWindowWidth() * listWidth, 0.0f), true, scrollBarWindowFlags);
		{
			for (int i = 0; i < licenseData.size(); i++)
			{
				if (Gui::Selectable(licenseData[i].Name.c_str(), i == selectedIndex))
					selectedIndex = i;
			}
		}
		Gui::EndChild();
		Gui::SameLine();
		Gui::BeginChild("InfoChild##LicenseWindow", vec2(0.0f, 0.0f), true);
		{
			if (selectedIndex >= 0 && selectedIndex < licenseData.size())
			{
				auto data = &licenseData[selectedIndex];

				Gui::BulletText("%s / %s:", data->Name.c_str(), data->LicenseName.c_str());
				Gui::Separator();

				Gui::BeginChild("InfoChildInner##LicenseWindow", vec2(0.0f, 0.0f), true);
				{
					Gui::TextUnformatted(data->Description.c_str());

					Gui::BeginChild("InfoChildLicense##LicenseWindow", vec2(0.0f, 0.0f), true, scrollBarWindowFlags);
					{
						Gui::TextUnformatted(data->License.c_str());

						if (!data->Remark.empty())
						{
							Gui::TextUnformatted("\n");
							Gui::Separator();
							Gui::PushStyleColor(ImGuiCol_Text, remarkTextColor);
							Gui::TextUnformatted(data->Remark.c_str());
							Gui::PopStyleColor();
						}
					}
					Gui::EndChild();
				}
				Gui::EndChild();
			}
		}
		Gui::EndChild();
		return true;
	}

	bool* LicenseWindow::GetIsWindowOpen()
	{
		return &isWindowOpen;
	}

	const char* LicenseWindow::GetWindowName() const
	{
		return "License Window";
	}

	void LicenseWindow::LoadLicenseData()
	{
		const auto licenseDirectoryEntry = ComfyData->FindDirectory(licenseDirectory);
		assert(licenseDirectoryEntry != nullptr);

		licenseData.reserve(licenseDirectoryEntry->EntryCount);

		for (size_t i = 0; i < licenseDirectoryEntry->EntryCount; i++)
		{
			const auto licenseFileEntry = licenseDirectoryEntry->Entries[i];

			std::unique_ptr<char[]> fileContent = std::make_unique<char[]>(licenseFileEntry.Size + 1);
			ComfyData->ReadEntryIntoBuffer(&licenseFileEntry, fileContent.get());

			const char* textBuffer = fileContent.get();
			const char* textBufferEnd = textBuffer + licenseFileEntry.Size;

			licenseData.emplace_back();
			auto info = &licenseData.back();

			enum { name, description, license_name, license, remark } type = {};

			while (true)
			{
				if (textBuffer >= textBufferEnd || textBuffer[0] == '\0')
					break;

				auto line = StringParsing::GetLineAdvanceToNextLine(textBuffer);

				if (StartsWith(line, '#'))
				{
					if (line == "#name")
						type = name;
					else if (line == "#description")
						type = description;
					else if (line == "#license_name")
						type = license_name;
					else if (line == "#license")
						type = license;
					else if (line == "#remark")
						type = remark;
					continue;
				}

				std::string& stringToAppend = info->Strings[type];
				stringToAppend.reserve(stringToAppend.size() + line.size() + 1);
				stringToAppend.append(line);
				stringToAppend.append("\n");
			}
		}

		for (auto& info : licenseData)
			info.TrimAllEnds();
	}
}
