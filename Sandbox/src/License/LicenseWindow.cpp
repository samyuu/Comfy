#include "LicenseWindow.h"
#include "ImGui/Gui.h"
#include "FileSystem/FileHelper.h"

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

	Gui::BeginChild("ListChild##LicenseWindow", ImVec2(Gui::GetWindowWidth() * listWidth, 0), true, scrollBarWindowFlags);
	{
		for (int i = 0; i < licenseData.size(); i++)
			if (Gui::Selectable(licenseData[i].Name.c_str(), i == selectedIndex))
				selectedIndex = i;
	}
	Gui::EndChild();
	Gui::SameLine();
	Gui::BeginChild("InfoChild##LicenseWindow", ImVec2(0, 0), true);
	{
		if (selectedIndex >= 0 && selectedIndex < licenseData.size())
		{
			auto data = &licenseData[selectedIndex];

			Gui::BulletText("%s / %s:", data->Name.c_str(), data->LicenseName.c_str());
			Gui::Separator();

			Gui::BeginChild("InfoChildInner##LicenseWindow", ImVec2(0, 0), true);
			{
				Gui::TextUnformatted(data->Description.c_str());

				Gui::BeginChild("InfoChildLicense##LicenseWindow", ImVec2(0, 0), true, scrollBarWindowFlags);
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
	auto licenseFilePaths = FileSystem::GetFiles("rom/license");
	licenseData.reserve(licenseFilePaths.size());

	for (const auto& filePath : licenseFilePaths)
	{
		std::vector<std::string> lines;
		FileSystem::ReadAllLines(filePath, &lines);

		licenseData.emplace_back();
		auto info = &licenseData.back();

		enum { name, description, license_name, license, remark } type = {};

		for (size_t i = 0; i < lines.size(); i++)
		{
			std::string& line = lines[i];

			if (line.size() > 0 && line.front() == '#')
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
