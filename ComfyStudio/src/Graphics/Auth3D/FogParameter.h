#pragma once
#include "Types.h"
#include "Graphics/GraphicsTypes.h"
#include "FileSystem/FileInterface.h"

namespace Graphics
{
	struct Fog
	{
		FogType Type;
		float Density;
		float Start;
		float End;
		vec3 Color;
	};

	class FogParameter final : public FileSystem::IBufferParsable
	{
	public:
		FogParameter();
		~FogParameter() = default;

	public:
		Fog Depth;
		Fog Height;
		Fog Bump;

	public:
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
	};
}
