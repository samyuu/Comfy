#pragma once
#include "Types.h"
#include "TimeSpan.h"

namespace Comfy
{
	constexpr const char* FileNameDateFormatString = "%Y-%m-%d_%H-%M-%S";

	// NOTE: For tagging file names with the current date
	size_t FormatFileNameDateTimeNow(char* outputBuffer, size_t bufferSize);
	std::string FormatFileNameDateTimeNow();
}
