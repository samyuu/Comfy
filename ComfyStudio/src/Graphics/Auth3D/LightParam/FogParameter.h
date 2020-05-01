#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Graphics
{
	struct Fog
	{
		FogType Type;
		float Density;
		float Start;
		float End;
		vec3 Color;
	};

	class FogParameter final : public IO::IBufferParsable
	{
	public:
		FogParameter();
		~FogParameter() = default;

	public:
		Fog Depth;
		Fog Height;
		Fog Bump;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
	};
}
