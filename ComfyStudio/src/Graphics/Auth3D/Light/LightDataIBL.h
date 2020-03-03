#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/Direct3D/D3D_Texture.h"
#include "FileSystem/FileInterface.h"

namespace Comfy::Graphics
{
	enum class LightMapFormat
	{
		RGBA8_CUBE,
		RGBA16F_CUBE,
		RGBA32F_CUBE,
	};

	struct LightMap
	{
		static constexpr size_t Faces = 6, MipMaps = 2;

		LightMapFormat Format;
		ivec2 Size;
		std::array<std::array<const uint8_t*, MipMaps>, Faces> DataPointers;

		UniquePtr<D3D_CubeMap> D3D_CubeMap;
	};

	struct LightData
	{
		vec3 LightDirection;
		vec3 LightColor;
		std::array<mat4, 3> IrradianceRGB;
	};

	class LightDataIBL final : public FileSystem::IBufferParsable
	{
	public:
		LightDataIBL();
		~LightDataIBL() = default;

	public:
		uint32_t Version;

		union
		{
			struct { LightData Character, Stage, Sun; };
			std::array<LightData, 3> Lights;
		};

		std::array<LightMap, 3> LightMaps;

	public:
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
		void UploadAll();
	};
}
