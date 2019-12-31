#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"

struct ComfyConfigData
{
	struct ComfyHeader
	{
		std::array<uint8_t, 4> Magic = { 0xCF, 0xCC, 0xAC, 0x90 };

		struct ComfyVersion
		{
			uint8_t Major = 0x01;
			uint8_t Minor = 0x00;
			uint16_t Reserved = 0xCCCC;
		} Version;

		std::array<uint8_t, 4> CreatorID = { 'c', 'm', 'f', 'y' };
		std::array<uint8_t, 4> ReservedID = { 0x90, 0x90, 0x90, 0x90 };

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
			std::array<int32_t, 3> Padding = { -1, -1, -1 };
		} Reserved;

	} Data;
};

constexpr std::string_view ComfyConfigFileName = "ComfyConfig.bin";

constexpr bool LoadComfyConfig = true;
constexpr bool SaveComfyConfig = true;

inline ComfyConfigData ComfyConfig;
