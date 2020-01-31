#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/Transform.h"

namespace Editor
{
	struct CharacterTestData
	{
		struct ItemIDs
		{
			int CommonItem = 1001;
			int Face = 0, FaceIndex = 0;
			// NOTE: item[0] range ~701
			int Overhead = -1; // 701;
			// NOTE: item[1] range ~501
			int Hair = 532;
			// NOTE: item[2] range ~1
			int Outer = 47;
			// NOTE: item[3] range ~301
			int Hands = 301;
	
			std::array<char, 8> Character = { "rin" };

		} IDs;

		Graphics::Transform Transform = Graphics::Transform(vec3(0.0f));
	};
}
