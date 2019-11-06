#pragma once
#include "Types.h"
#include "Misc/StringHelper.h"
#include "Core/CoreTypes.h"

struct LicenseInfo
{
	union
	{
		struct
		{
			std::string Name;
			std::string Description;
			std::string LicenseName;
			std::string License;
			std::string Remark;
		};
		std::string Strings[5];
	};

	LicenseInfo() : LicenseInfo("", "", "", "", "")
	{
	}

	LicenseInfo(const char* name, const char* description, const char* licenseName, const char* license, const char* remark)
		: Name(name), Description(description), LicenseName(licenseName), License(license), Remark(remark)
	{
		TrimAllEnds();
	}

	LicenseInfo(const LicenseInfo& other)
	{
		*Strings = *other.Strings;
	}

	~LicenseInfo()
	{
	}

	void TrimAllEnds()
	{
		for (auto& string : Strings)
			Trim(string);
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
	std::vector<LicenseInfo> licenseData;

	static constexpr float listWidth = 0.2f;
	static constexpr const vec4 remarkTextColor = vec4(0.85f, 0.86f, 0.15f, 1.0f);
	
	static constexpr const char* licenseDirectory = "license";

	void LoadLicenseData();
};