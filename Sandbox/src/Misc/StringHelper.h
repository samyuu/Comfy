#pragma once
#include "Core/CoreTypes.h"

void TrimLeft(String &string);
void TrimRight(String &string);
void Trim(String &string);

bool StartsWith(const String& string, const String& prefix);
bool StartsWith(const WideString& string, const WideString& prefix);

bool StartsWithInsensitive(const String& string, const String& prefix);
bool StartsWithInsensitive(const WideString& string, const WideString& prefix);

bool EndsWith(const String& string, char suffix);
bool EndsWith(const WideString& string, wchar_t suffix);

bool EndsWith(const String& string, const String& suffix);
bool EndsWith(const WideString& string, const WideString& suffix);

bool EndsWithInsensitive(const String& string, const String& suffix);
bool EndsWithInsensitive(const WideString& string, const WideString& suffix);

WideString Utf8ToUtf16(const String& string);
String Utf16ToUtf8(const WideString& string);
