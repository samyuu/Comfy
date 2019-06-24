#include "StringHelper.h"

bool CaseInsenitiveComparison(char a, char b)
{
	return tolower(a) == tolower(b);
}

bool CaseInsenitiveWideComparison(wchar_t a, wchar_t b)
{ 
	return tolower(a) == tolower(b); 
};

bool EndsWith(const std::string& string, const std::string& ending)
{
	if (ending.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + ending.size(), ending.rbegin(), ending.rend());
}

bool EndsWith(const std::wstring& string, const std::wstring& ending)
{
	if (ending.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + ending.size(), ending.rbegin(), ending.rend());
}

bool EndsWithCaseInsensitive(const std::string& string, const std::string& ending)
{
	if (ending.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + ending.size(), ending.rbegin(), ending.rend(), CaseInsenitiveComparison);
}

bool EndsWithCaseInsensitive(const std::wstring& string, const std::wstring& ending)
{
	if (ending.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + ending.size(), ending.rbegin(), ending.rend(), CaseInsenitiveWideComparison);
}
