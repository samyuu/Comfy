#include "LicenseWindow.h"
#include "ImGui/Gui.h"
#include "IO/File.h"
#include "Misc/StringParseHelper.h"
#include "Misc/StringUtil.h"
#include "System/ComfyData.h"

namespace Comfy::Studio
{
	namespace
	{
		inline std::array<std::string*, 5> LicenseInfoStringPointers(LicenseInfo& info)
		{
			return { &info.Name, &info.Description, &info.LicenseName, &info.License, &info.Remark };
		}
	}

	bool LicenseWindow::DrawGui()
	{
		if (!dataLoaded)
		{
			LoadLicensesData();
			dataLoaded = true;
		}

		constexpr ImGuiWindowFlags scrollBarWindowFlags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

		const auto listWidth = 180.0f; // Gui::GetWindowWidth() * 0.2f;
		Gui::BeginChild("ListChild##LicenseWindow", vec2(listWidth, 0.0f), true, scrollBarWindowFlags);
		{
			for (int i = 0; i < licenses.size(); i++)
			{
				if (Gui::Selectable(licenses[i].Name.c_str(), i == selectedIndex))
					selectedIndex = i;
			}
		}
		Gui::EndChild();
		Gui::SameLine();
		Gui::BeginChild("InfoChild##LicenseWindow", vec2(0.0f, 0.0f), true);
		{
			if (selectedIndex >= 0 && selectedIndex < licenses.size())
			{
				auto data = &licenses[selectedIndex];

				Gui::AlignTextToFramePadding();
				Gui::BulletText("%s / %s:", data->Name.c_str(), data->LicenseName.c_str());
				Gui::Separator();

				Gui::BeginChild("InfoChildInner##LicenseWindow", vec2(0.0f, 0.0f), true);
				{
					Gui::AlignTextToFramePadding();
					Gui::TextUnformatted(data->Description.c_str());
					
					Gui::BeginChild("InfoChildLicense##LicenseWindow", vec2(0.0f, 0.0f), true, scrollBarWindowFlags);
					{
						Gui::AlignTextToFramePadding();
						Gui::TextUnformatted(data->License.c_str());

						if (!data->Remark.empty())
						{
							Gui::TextUnformatted("\n");
							Gui::Separator();
							Gui::PushStyleColor(ImGuiCol_Text, vec4(0.85f, 0.86f, 0.15f, 1.0f));
							Gui::AlignTextToFramePadding();
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

	const char* LicenseWindow::GetWindowName() const
	{
		return "License Info";
	}

	void LicenseWindow::LoadLicensesData()
	{
		const auto licenseDirectoryEntry = System::Data.FindDirectory("license");
		if (licenseDirectoryEntry == nullptr)
			return;

		licenses.reserve(licenseDirectoryEntry->EntryCount);

		size_t largestFileSize = 0;
		for (size_t i = 0; i < licenseDirectoryEntry->EntryCount; i++)
			largestFileSize = std::max(largestFileSize, licenseDirectoryEntry->Entries[i].Size);
		auto fileContentBuffer = std::make_unique<char[]>(largestFileSize + 1);

		for (size_t i = 0; i < licenseDirectoryEntry->EntryCount; i++)
		{
			const auto licenseFileEntry = licenseDirectoryEntry->Entries[i];
			System::Data.ReadFileIntoBuffer(&licenseFileEntry, fileContentBuffer.get());
			fileContentBuffer[licenseFileEntry.Size] = '\0';

			const char* textBuffer = fileContentBuffer.get();
			const char* textBufferEnd = textBuffer + licenseFileEntry.Size;

			licenses.emplace_back();
			auto license = &licenses.back();

			enum { Name, Description, LicenseName, LicenseText, Remark } type = {};
			while (true)
			{
				if (textBuffer >= textBufferEnd || textBuffer[0] == '\0')
					break;

				const auto line = Util::StringParsing::GetLineAdvanceToNextLine(textBuffer);
				if (Util::StartsWith(line, '#'))
				{
					if (line == "#name")
						type = Name;
					else if (line == "#description")
						type = Description;
					else if (line == "#license_name")
						type = LicenseName;
					else if (line == "#license")
						type = LicenseText;
					else if (line == "#remark")
						type = Remark;
					continue;
				}

				auto& stringToAppend = *LicenseInfoStringPointers(*license)[type];
				stringToAppend.reserve(stringToAppend.size() + line.size() + 1);
				stringToAppend.append(line);
				stringToAppend.append("\n");
			}
		}

		for (auto& license : licenses)
		{
			for (auto* string : LicenseInfoStringPointers(license))
				*string = Util::Trim(*string);
		}
	}
}
