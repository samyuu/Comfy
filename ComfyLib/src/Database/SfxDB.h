#pragma once
#include "Database.h"

namespace Comfy::Database
{
	struct SfxEntry
	{
		std::string FileName;
		std::string Name;
		f32 Volume;
		i32 LoopStartFrame;
		i32 LoopEndFrame;
		i32 ReleaseFrame;
	};

	// NOTE: Sound Effect FArc property.txt index file
	class SfxDB final : public TextDatabase
	{
	public:
		std::vector<SfxEntry> Entries;
		std::string VersionDate;
		std::string VersionName;
		f32 VolumeBias;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;

	private:
	};
}
