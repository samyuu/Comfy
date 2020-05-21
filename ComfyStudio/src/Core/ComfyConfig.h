#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Studio
{
	struct ComfyConfigData
	{
		struct ComfyHeader
		{
			std::array<u8, 4> Magic = { 0xCF, 0xCC, 0xAC, 0x90 };

			struct ComfyVersion
			{
				u8 Major = 0x01;
				u8 Minor = 0x00;
				u16 Reserved = 0xCCCC;
			} Version;

			std::array<u8, 4> CreatorID = { 'c', 'm', 'f', 'y' };
			std::array<u8, 4> ReservedID = { 0x90, 0x90, 0x90, 0x90 };

		} Header;

		struct Data
		{
			struct WindowData
			{
				ivec4 RestoreRegion = ivec4(0, 0, 16, 16);
				ivec2 Position = ivec2(0, 0);
				ivec2 Size = ivec2(16, 16);

				bool Fullscreen = false;
				bool Maximized = false;
			} Window;

			struct ReservedData
			{
				std::array<i32, 3> Padding = { -1, -1, -1 };
			} Reserved;

		} Data;
	};

	constexpr std::string_view ComfyConfigFileName = "ComfyConfig.bin";

	constexpr bool LoadComfyConfig = true;
	constexpr bool SaveComfyConfig = true;

	inline ComfyConfigData ComfyConfig;
}
