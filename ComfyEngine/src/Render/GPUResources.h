// GPUResources.h
#pragma once
#include "Types.h"

#ifdef COMFY_D3D11
#include "Graphics/D3D11/Texture/Texture.h"
#include "Graphics/D3D11/Texture/RenderTarget.h"
#include "Graphics/D3D11/Buffer/IndexBuffer.h"
#include "Graphics/D3D11/Buffer/VertexBuffer.h"
#endif

namespace Comfy::Render
{
	// NOTE: GPU resource aliases to hide away the graphics API implementation without relying on virtual interfaces, for now

#ifdef COMFY_D3D11
	using GPU_Texture2D = D3D11::Texture2D;
	using GPU_CubeMap = D3D11::CubeMap;
	using GPU_RenderTarget = D3D11::RenderTarget;
	using GPU_IndexBuffer = D3D11::StaticIndexBuffer;
	using GPU_VertexBuffer = D3D11::StaticVertexBuffer;
#else
	class GPU_Texture2D {};
	class GPU_CubeMap {};
	class GPU_RenderTarget {};
	class GPU_IndexBuffer {};
	class GPU_VertexBuffer {};
#endif

	struct Tex;
	struct LightMapIBL;

	namespace GPU
	{
		std::unique_ptr<GPU_Texture2D> MakeTexture2D(const Tex& tex, const char* debugName = nullptr);
		std::unique_ptr<GPU_Texture2D> MakeTexture2D(ivec2 size, const u32* rgbaBuffer, const char* debugName = nullptr);

		std::unique_ptr<GPU_CubeMap> MakeCubeMap(const Tex& tex, const char* debugName = nullptr);
		std::unique_ptr<GPU_CubeMap> MakeCubeMap(const LightMapIBL& lightMap, const char* debugName = nullptr);

		std::unique_ptr<GPU_IndexBuffer> MakeIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, const char* debugName = nullptr);
		std::unique_ptr<GPU_VertexBuffer> MakeVertexBuffer(size_t dataSize, const void* data, size_t stride, const char* debugName = nullptr);
	}
}


// GPURenderers.h
#pragma once
#include "Types.h"

#ifdef COMFY_D3D11
#include "Graphics/D3D11/Renderer/Renderer2D.h"
#include "Graphics/D3D11/Renderer/Renderer3D.h"
#endif

namespace Comfy::Render
{
	// TODO: Hide renderers behind proper interfaces

#ifdef COMFY_D3D11
	using GPU_Renderer2D = D3D11::Renderer2D;
	using GPU_Renderer3D = D3D11::Renderer3D;
#else
	class GPU_Renderer2D {};
	class GPU_Renderer3D {};
#endif
}
