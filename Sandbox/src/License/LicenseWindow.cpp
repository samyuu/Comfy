#include "LicenseWindow.h"
#include "ImGui/imgui.h"
#include "FileSystem/FileHelper.h"

bool LicenseWindow::DrawGui()
{
	if (!initialized)
	{
		LoadLicenseData();
		initialized = true;
	}

	constexpr ImGuiWindowFlags scrollBarWindowFlags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

	ImGui::BulletText("License:");
	ImGui::Separator();

	ImGui::BeginChild("##LicenseWindowListChild", ImVec2(ImGui::GetWindowWidth() * listWidth, 0), true, scrollBarWindowFlags);
	{
		for (size_t i = 0; i < licenseData.size(); i++)
			if (ImGui::Selectable(licenseData[i].Name.c_str(), i == selectedIndex))
				selectedIndex = i;
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##LicenseWindowInfoChild", ImVec2(0, 0), true);
	{
		if (selectedIndex >= 0 && selectedIndex < licenseData.size())
		{
			auto data = &licenseData[selectedIndex];

			ImGui::BulletText("%s / %s:", data->Name.c_str(), data->LicenseName.c_str());
			ImGui::Separator();

			ImGui::BeginChild("##LicenseWindowInfoChildInner", ImVec2(0, 0), true);
			{
				ImGui::Text("%s", data->Description.c_str());

				ImGui::BeginChild("##LicenseWindowInfoChildLicense", ImVec2(0, 0), true, scrollBarWindowFlags);
				{
					ImGui::Text("%s", data->License.c_str());
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
	}
	ImGui::EndChild();
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
	auto licenseFilePaths = FileSystem::GetFiles(licenseDirectory);

	for (const auto& filePath : licenseFilePaths)
	{
		std::vector<std::string> lines;
		FileSystem::ReadAllLines(filePath, &lines);

		licenseData.emplace_back();
		auto info = &licenseData.back();

		enum { name, description, license_name, license } type = {};

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
				continue;
			}

			std::string* stringToAppend = &(&info->Name)[type];

			stringToAppend->reserve(stringToAppend->size() + line.size() + 1);
			stringToAppend->append(line);
			stringToAppend->append("\n");
		}
	}

	for (auto& info : licenseData)
		info.TrimAllEnds();
}
