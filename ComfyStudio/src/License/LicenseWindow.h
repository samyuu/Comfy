#pragma once
#include "Types.h"

namespace Comfy::Studio
{
	struct LicenseInfo
	{
		std::string Name;
		std::string Description;
		std::string LicenseName;
		std::string License;
		std::string Remark;
	};

	class LicenseWindow
	{
	public:
		LicenseWindow() = default;
		~LicenseWindow() = default;

	public:
		bool DrawGui();
		const char* GetWindowName() const;

	private:
		void LoadLicensesData();

	private:
		bool dataLoaded = false;
		int selectedIndex = 0;
		std::vector<LicenseInfo> licenses;
	};
}
