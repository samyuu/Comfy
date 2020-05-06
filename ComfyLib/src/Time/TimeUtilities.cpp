#include "TimeUtilities.h"
#include <ctime>

namespace Comfy
{
	size_t FormatFileNameDateTimeNow(char* outputBuffer, size_t bufferSize)
	{
		const auto timeNow = std::time(nullptr);

		tm dateNow;
		const auto error = localtime_s(&dateNow, &timeNow);

		return std::strftime(outputBuffer, bufferSize, FileNameDateFormatString, &dateNow);
	}

	std::string FormatFileNameDateTimeNow()
	{
		char buffer[64];
		const auto stringLength = FormatFileNameDateTimeNow(buffer, sizeof(buffer));

		return std::string(buffer, stringLength);
	}
}
