#pragma once
#include "Types.h"
#include "Time/TimeSpan.h"
#include "Time/Stopwatch.h"

#define COMFY_PROFILER_ENABLED 0

#if COMFY_PROFILER_ENABLED
#define ProfileFunction() Comfy::System::ProfilerRAII COMFY_UNIQUENAME(__PROFILER_ENTRY) (__FUNCTION__)
#else
#define ProfileFunction() do { } while(false)
#endif

namespace Comfy::System
{
	class Profiler
	{
	public:
		// NOTE: So we know when to swap our entry stacks
		void StartFrame();
		void EndFrame();

		void PushEntry(const char* name);
		void PopEntry();

		static inline Profiler& Get() { return instance; }

	protected:
		struct Entry
		{
			Entry(const char* name, TimeSpan startTime, i32 parentCount);

			// NOTE: Should be set using a function name macro
			const char* Name;

			// NOTE: Should be set using TimeSpan::GetTimeNow
			TimeSpan StartTime, EndTime;

			// NOTE: Basically the "indentation" of this entry
			i32 ParentCount;

			inline TimeSpan Duration() const { return EndTime - StartTime; }
		};

		i32 currentDepth = 0;

		// NOTE: Time set by StartFrame / EndFrame
		TimeSpan frameStartTime, frameEndTime;

		// NOTE: Always keep track of the entries from the previous entries for visualization
		std::vector<Entry> entries, previousEntries;

	private:
		// NOTE: The profiler should only ever be accessed from the main thread and stores minimal state sso this shouldn't cause any problems
		static Profiler instance;
	};

	struct ProfilerRAII
	{
		ProfilerRAII(const char* name);
		~ProfilerRAII();

		inline Profiler& GetProfiler() const { return Profiler::Get(); }
	};
}
