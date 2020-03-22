#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/GPU/GPUResources.h"
#include "FileSystem/FileInterface.h"

namespace Comfy::Graphics
{
	enum class LightMapFormat
	{
		RGBA8_CUBE,
		RGBA16F_CUBE,
		RGBA32F_CUBE,
	};

	struct LightMapIBL
	{
		static constexpr size_t Faces = 6, MipMaps = 2;

		LightMapFormat Format;
		ivec2 Size;
		std::array<std::array<const uint8_t*, MipMaps>, Faces> DataPointers;

		UniquePtr<GPU_CubeMap> GPU_CubeMap;
	};

	struct LightDataIBL
	{
		vec3 LightDirection;
		vec3 LightColor;
		std::array<mat4, 3> IrradianceRGB;
	};

	class IBLParameters final : public FileSystem::IBufferParsable
	{
	public:
		IBLParameters();
		~IBLParameters() = default;

	public:
		uint32_t Version;

		union
		{
			// TODO: Decide on a final name
			// struct { LightDataIBL Specular, Diffuse, Back; };
			// struct { LightDataIBL Character, Stage, Sun; };
			std::array<LightDataIBL, 3> Lights;
		};

		std::array<LightMapIBL, 3> LightMaps;

	public:
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
		void UploadAll();
	};
}
