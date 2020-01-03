#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/Direct3D/D3D_Texture.h"
#include "FileSystem/FileInterface.h"

namespace Graphics
{
	enum class LightMapFormat
	{
		RGBA8_CUBE,
		RGBA16F_CUBE,
		RGBA32F_CUBE,
	};

	struct LightMap
	{
		LightMapFormat Format;
		ivec2 Size;
		std::array<const uint8_t*, 6> DataPointers;

		UniquePtr<D3D_CubeMap> CubeMap;
	};

	struct LightData
	{
		vec3 LightDirection;
		vec3 LightColor;
		std::array<mat4, 3> IrradianceRGB;
		LightMap LightMap;
	};

	class LightDataIBL final : public FileSystem::IBufferParsable
	{
	public:
		LightDataIBL();
		~LightDataIBL() = default;

	public:
		uint32_t Version;
	
		LightData Character;
		LightData Stage;
		LightData Sun;
		LightData Reflect;
		LightData Shadow;
		LightData CharacterColor;
		LightData CharacterF;
		LightData Projection;

		LightData* GetLightData(LightTargetType type);

	public:
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
		void UploadAll();
	};
}
