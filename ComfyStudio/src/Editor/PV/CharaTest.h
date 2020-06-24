#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/Transform.h"

namespace Comfy::Studio::Editor
{
	struct CharacterTestItemIDs
	{
		int CommonItem = 1001;
		int Face = 0;
		int FaceIndex = 0;
		// NOTE: item[0] range ~701
		int Overhead = -1; // 701;
		// NOTE: item[1] range ~501
		int Hair = 532;
		// NOTE: item[2] range ~1
		int Outer = 47;
		// NOTE: item[3] range ~301
		int Hands = 301;

		std::array<char, 8> Character = { "rin" };
	};

	static constexpr std::array CharacterTestItemPresets =
	{
		CharacterTestItemIDs { 1001, 0, 0, -1, 532,  47, 301, { "rin" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 532,  32, 301, { "rin" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 532,   2, 301, { "rin" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 554,   1, 301, { "mik" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 533,  61, 301, { "mik" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 633, 114, 301, { "mik" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 533, 152, 301, { "mik" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 537,  37, 301, { "luk" } },
		CharacterTestItemIDs { 1001, 0, 0, -1, 537,  21, 301, { "luk" } },
	};

	struct CharacterTestData
	{
		CharacterTestItemIDs IDs = CharacterTestItemPresets[0];

		Graphics::Transform Transform = Graphics::Transform(vec3(0.0f));
	};
}
