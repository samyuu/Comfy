#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Editor
{
	struct CharacterTestData
	{
		struct ItemIDs
		{
			// NOTE: item[0] range ~701
			int Overhead = 701;
			// NOTE: item[1] range ~501
			int Hair = 532;
			// NOTE: item[2] range ~1
			int Outer = 47;
			// NOTE: item[3] range ~301
			int Hands = 301;
	
			std::array<char, 8> Character = { "rin" };

		} IDs;

		vec3 Position = vec3(0.0f, 0.0f, 0.0f);

	};
}
