#pragma once

class LicenseWindow
{
public:
	bool DrawGui();
	bool* GetIsOpen();
	const char* GetWindowName() const;

private:
	bool isOpen = true;
	int selectedIndex = 0;
};