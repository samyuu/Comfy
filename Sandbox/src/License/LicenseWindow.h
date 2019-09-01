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
			String Name;
			String Description;
			String LicenseName;
			String License;
			String Remark;
		};
		String Strings[5];
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
	const float listWidth = .2f;

	Vector<LicenseInfo> licenseData;
	const vec4 remarkTextColor = { 0.85f, 0.86f, 0.15f, 1.0f };

	void LoadLicenseData();
};