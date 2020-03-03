#include "Profiler.h"

namespace Comfy::System
{
	Profiler Profiler::instance;

	void Profiler::StartFrame()
	{
#if !COMFY_PROFILER_ENABLED
		return;
#endif

		previousEntries = entries;
		entries.clear();

		frameStartTime = TimeSpan::GetTimeNow();
		currentDepth = 0;
	}
	
	void Profiler::EndFrame()
	{
#if !COMFY_PROFILER_ENABLED
		return;
#endif

		frameEndTime = TimeSpan::GetTimeNow();
	}

	void Profiler::PushEntry(const char* name)
	{
		TimeSpan startTime = TimeSpan::GetTimeNow() - frameStartTime;
		entries.emplace_back(name, startTime, currentDepth);

		++currentDepth;
	}
	
	void Profiler::PopEntry()
	{
		if (entries.empty())
		{
			assert(false);
			return;
		}

		TimeSpan endTime = TimeSpan::GetTimeNow();
		entries[entries.size() - currentDepth].EndTime = endTime - frameStartTime;
		--currentDepth;
	}
	
	Profiler::Entry::Entry(const char* name, TimeSpan startTime, int32_t parentCount)
		: Name(name), StartTime(startTime), EndTime(0.0), ParentCount(parentCount)
	{
	}

	ProfilerRAII::ProfilerRAII(const char* name)
	{
		GetProfiler().PushEntry(name);
	}

	ProfilerRAII::~ProfilerRAII()
	{
		GetProfiler().PopEntry();
	}
}
