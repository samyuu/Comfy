#pragma once
#include "Misc/StringHelper.h"
#include <vector>

struct LicenseInfo
{
	std::string Name;
	std::string Description;
	std::string LicenseName;
	std::string License;

	LicenseInfo()
	{
	};

	LicenseInfo(const char* name, const char* description, const char* licenseName, const char* license)
		: Name(name), Description(description), LicenseName(licenseName), License(license)
	{
		TrimAllEnds();
	};

	void TrimAllEnds()
	{
		for (std::string* stringPtr = &Name; stringPtr <= &License; stringPtr++)
			Trim(*stringPtr);
	}
};

class LicenseWindow
{
public:
	bool DrawGui();
	bool* GetIsWindowOpen();
	const char* GetWindowName() const;

private:
	bool dataLoaded = false;
	bool isWindowOpen = true;
	int selectedIndex = 0;
	const float listWidth = .2f;

	std::vector<LicenseInfo> licenseData;

	void LoadLicenseData();
};