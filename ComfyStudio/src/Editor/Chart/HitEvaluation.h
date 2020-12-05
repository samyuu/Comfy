#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Button type hit evaluation behavior
	/*
	Matching type:
		Cool (Gold), Fine (Silver), Safe (Green), Sad (Blue)

	Mismatching type (including sync and slide after timeout if wrong direction):
		Wrong (Red), Wrong (Gray), Wrong (Green), Wrong (Blue)

	Mismatching type (sync assist enabled - "if at least one target in the sync pair was correctly hit", supposed to be "the right number of buttons were pressed"):
		Almost (Red), Almost (Gray), Almost (Green), Almost (Blue)

	Timeout:
		Worst (Purple)
	*/

	enum class HitEvaluation : u8
	{
		None,

		Cool,
		Fine,
		Safe,
		Sad,

		WrongCool,
		WrongFine,
		WrongSafe,
		WrongSad,

		AlmostCool,
		AlmostFine,
		AlmostSafe,
		AlmostSad,

		Worst,
		Count
	};

	enum class HitPrecision : u8
	{
		Early,
		Just,
		Late,
		Count
	};

	namespace HitThreshold
	{
		constexpr TimeSpan Cool = TimeSpan::FromMilliseconds(30.0);
		constexpr TimeSpan CoolSlide = TimeSpan::FromMilliseconds(45.0);
		constexpr TimeSpan Fine = TimeSpan::FromMilliseconds(70.0);
		constexpr TimeSpan FineSlide = TimeSpan::FromMilliseconds(130.0);
		constexpr TimeSpan Safe = TimeSpan::FromMilliseconds(100.0);
		constexpr TimeSpan Sad = TimeSpan::FromMilliseconds(130.0);
		constexpr TimeSpan Worst = TimeSpan::FromMilliseconds(100.0);

		constexpr f32 ChainSlidePreHitProgress = (1.0f - (1.0f / 8.0f));

		constexpr HitPrecision EvaluatePrecision(TimeSpan remaining, TimeSpan threshold)
		{
			return
				(remaining >= (threshold * -(1.0 / 3.0)) && remaining <= (threshold * +(1.0 / 3.0))) ? HitPrecision::Just :
				(remaining > TimeSpan::Zero()) ? HitPrecision::Early : HitPrecision::Late;
		}
	}
}
