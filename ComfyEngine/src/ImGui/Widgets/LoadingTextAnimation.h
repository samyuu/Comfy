#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"

namespace ImGui
{
	class LoadingTextAnimation
	{
	public:
		virtual ~LoadingTextAnimation() = default;

	public:
		void Reset();
		void Update();

		virtual const char* GetText() const = 0;

	protected:
		virtual void OnReset() = 0;
		virtual void OnUpdate(Comfy::TimeSpan elapsed) = 0;

	private:
		Comfy::TimeSpan currentTime, lastTime;
	};

	class BinaryLoadingTextAnimation final : public LoadingTextAnimation
	{
	public:
		BinaryLoadingTextAnimation() = default;
		~BinaryLoadingTextAnimation() = default;

	public:
		const char* GetText() const override;

	protected:
		void OnReset() override;
		void OnUpdate(Comfy::TimeSpan elapsed) override;

	private:
		static constexpr auto textPrefix = std::string_view { "  Loading  " };
		static constexpr char onBitChar = '.', offBitChar = ' ';

		Comfy::TimeSpan bitTickInterval = Comfy::TimeSpan::FromMilliseconds(39.0f);
		Comfy::TimeSpan elapsedTime = Comfy::TimeSpan::Zero();

		u16 elapsedBits = 0;
		u8 usedBitCount = 12;
		u8 bitShiftDirection = 1;

		enum class AnimationType : u8
		{
			// NOTE: Seems to be the most pleasant of the options
			BitIncrement,
			// NOTE: The wrap around jump looks a bit jarring
			BitShiftLeft,
			// NOTE: Better than a wrap around but still not perfect
			BitShiftPingPong,
		} Animation = AnimationType::BitIncrement;

		std::array<char, textPrefix.size() + (sizeof(elapsedBits) * 8) + 1> textBuffer;
	};
}
