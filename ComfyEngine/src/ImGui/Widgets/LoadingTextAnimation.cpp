#include "LoadingTextAnimation.h"

using namespace Comfy;

namespace ImGui
{
	void LoadingTextAnimation::Reset()
	{
		lastTime = currentTime;
		OnReset();
	}

	void LoadingTextAnimation::Update()
	{
		currentTime = TimeSpan::GetTimeNow();
		const auto elapsed = currentTime - lastTime;
		lastTime = currentTime;

		OnUpdate(elapsed);
	}

	const char* BinaryLoadingTextAnimation::GetText() const
	{
		return textBuffer.data();
	}

	void BinaryLoadingTextAnimation::OnReset()
	{
		elapsedTime = TimeSpan::Zero();
		elapsedBits = 0;
	}

	void BinaryLoadingTextAnimation::OnUpdate(TimeSpan elaped)
	{
		assert(usedBitCount <= (sizeof(elapsedBits) * CHAR_BIT));

		if (elapsedTime > bitTickInterval)
		{
			elapsedTime -= bitTickInterval;

			if (Animation == AnimationType::BitIncrement)
			{
				elapsedBits++;
			}
			else if (Animation == AnimationType::BitShiftLeft)
			{
				if ((elapsedBits == 0) || (elapsedBits & (1 << usedBitCount)))
					elapsedBits = 1;
				else
					elapsedBits <<= 1;
			}
			else if (Animation == AnimationType::BitShiftPingPong)
			{
				if (elapsedBits & (1 << usedBitCount))
					bitShiftDirection ^= 1;
				else if (elapsedBits == 0)
					elapsedBits = (bitShiftDirection ^= 1);
				
				elapsedBits = bitShiftDirection ? (elapsedBits << 1) : (elapsedBits >> 1);
			}
		}

		elapsedTime += elaped;

		for (size_t i = 0; i < textPrefix.size(); i++)
			textBuffer[i] = textPrefix[i];

		for (u32 bitIndex = 0; bitIndex < usedBitCount; bitIndex++)
			textBuffer[textPrefix.size() + bitIndex] = (elapsedBits & (1 << bitIndex)) ? onBitChar : offBitChar;

		textBuffer[textPrefix.size() + usedBitCount] = '\0';
	}
}
