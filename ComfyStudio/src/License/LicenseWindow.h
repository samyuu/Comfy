#pragma once
#include "Types.h"
#include "Misc/StringUtil.h"
#include "CoreTypes.h"

namespace Comfy::Studio
{
	struct LicenseInfo
	{
		std::string Name;
		std::string Description;
		std::string LicenseName;
		std::string License;
		std::string Remark;

		std::array<std::string*, 5> GetStrings()
		{
			return { &Name, &Description, &LicenseName, &License, &Remark };
		}

		LicenseInfo() : LicenseInfo("", "", "", "", "")
		{
		}

		LicenseInfo(std::string_view name, std::string_view description, std::string_view licenseName, std::string_view license, std::string_view remark)
			: Name(name), Description(description), LicenseName(licenseName), License(license), Remark(remark)
		{
			TrimAllEnds();
		}

		void TrimAllEnds()
		{
			for (auto* string : GetStrings())
				*string = std::string(Util::Trim(*string));
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
}
