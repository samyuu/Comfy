#pragma once
#include "Database/Database.h"

namespace Comfy::Database
{
	enum class GmBtnSfxType : u8
	{
		Button,
		Slide,
		ChainSlide,
		SliderTouch,
		Count
	};

	struct GmBtnSfxEntry
	{
		DateEntry StartDate, EndDate;
		u32 ID, SortIndex;
		std::string DisplayName;
		std::string SfxName;
		struct ChainData
		{
			std::string SfxNameFirst;
			std::string SfxNameSub;
			std::string SfxNameSuccess;
			std::string SfxNameFailure;
		} Chain;
	};

	class GmBtnSfxDB final : public TextDatabase
	{
	public:
		std::vector<GmBtnSfxEntry> Entries;
		GmBtnSfxType Type;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;

	private:
	};
}
