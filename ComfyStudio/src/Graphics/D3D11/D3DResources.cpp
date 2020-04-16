#pragma once
#include "Graphics/GPU/GPUResources.h"

namespace Comfy::Graphics::GPU
{
	namespace
	{
#if COMFY_D3D11_DEBUG_NAMES
		template <typename T>
		void SetDebugName(T& resource, const char* debugName)
		{
			if (debugName == nullptr)
				return;

			if constexpr (std::is_base_of<D3D11::TextureResource, T>::value)
			{
				char viewDebugName[128];
				sprintf_s(viewDebugName, "%s (RV)", debugName);

				D3D11_SetObjectDebugName(resource.GetTexture(), debugName);
				D3D11_SetObjectDebugName(resource.GetResourceView(), viewDebugName);
			}
			else
			{
				D3D11_SetObjectDebugName(resource.GetBuffer(), debugName);
			}
		}
#else
#define SetDebugName(...) /* ... */
#endif
	}

	UniquePtr<GPU_Texture2D> MakeTexture2D(const Tex& tex, const char* debugName)
	{
		auto result = MakeUnique<D3D11::Texture2D>(tex);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_Texture2D> MakeTexture2D(ivec2 size, const uint32_t* rgbaBuffer, const char* debugName)
	{
		auto result = MakeUnique<D3D11::Texture2D>(size, rgbaBuffer);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_CubeMap> MakeCubeMap(const Tex& tex, const char* debugName)
	{
		auto result = MakeUnique<D3D11::CubeMap>(tex);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_CubeMap> MakeCubeMap(const LightMapIBL& lightMap, const char* debugName)
	{
		auto result = MakeUnique<D3D11::CubeMap>(lightMap);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_IndexBuffer> MakeIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, const char* debugName)
	{
		auto result = MakeUnique<D3D11::StaticIndexBuffer>(dataSize, data, indexFormat);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_VertexBuffer> MakeVertexBuffer(size_t dataSize, const void* data, size_t stride, const char* debugName)
	{
		auto result = MakeUnique<D3D11::StaticVertexBuffer>(dataSize, data, stride);
		SetDebugName(*result, debugName);
		return result;
	}
}
