#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/GPUResource.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Graphics
{
	enum class LightMapFormat
	{
		RGBA8_CUBE,
		RGBA16F_CUBE,
		RGBA32F_CUBE,
	};

	// TODO: Replace with Tex to seemlessly integrate into the rest of the render APIs, add LightMapFormat to TextureFormats as special negative values
	struct LightMapIBL
	{
		static constexpr size_t Faces = 6, MipMaps = 2;

		LightMapFormat Format;
		ivec2 Size;

		// HACK: Point into IBLParameters::LightMapBinaryData
		std::array<std::array<const u8*, MipMaps>, Faces> DataPointers;

		InternallyManagedGPUResource GPU_CubeMap;
	};

	struct LightDataIBL
	{
		vec3 LightDirection;
		vec3 LightColor;
		std::array<mat4, 3> IrradianceRGB;
	};

	class IBLParameters final : public IO::IBufferParsable, NonCopyable
	{
	public:
		IBLParameters();
		~IBLParameters() = default;

	public:
		u32 Version;

		union
		{
			// TODO: Decide on a final name
			// struct { LightDataIBL Specular, Diffuse, Back; };
			// struct { LightDataIBL Character, Stage, Sun; };
			std::array<LightDataIBL, 3> Lights;
		};

		/* // NOTE: Storage layout:
			
			LightMaps[0].MipMaps[0] = Diffuse Irradiance
			LightMaps[0].MipMaps[1] = Diffuse Irradiance (Shadow)

			LightMaps[1].MipMaps[0] = Specular Radiance Shiny
			LightMaps[1].MipMaps[1] = Specular Radiance Rough
			
			LightMaps[2].MipMaps[0] = Specular Radiance Shiny (Shadow)
			LightMaps[2].MipMaps[1] = Specular Radiance Rough (Shadow)
		*/
		std::array<LightMapIBL, 3> LightMaps;

		// HACK: Owning binary data pointed into by LightMapIBL::DataPointers
		std::unique_ptr<u8[]> LightMapBinaryData;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
	};
}
