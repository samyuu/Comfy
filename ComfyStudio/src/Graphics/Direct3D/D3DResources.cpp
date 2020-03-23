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

			if constexpr (std::is_base_of<D3D_TextureResource, T>::value)
			{
				char viewDebugName[128];
				sprintf_s(viewDebugName, "%s (RV)", debugName);

				D3D_SetObjectDebugName(resource.GetTexture(), debugName);
				D3D_SetObjectDebugName(resource.GetResourceView(), viewDebugName);
			}
			else
			{
				D3D_SetObjectDebugName(resource.GetBuffer(), debugName);
			}
		}
#else
#define SetDebugName(...) /* ... */
#endif
	}

	UniquePtr<GPU_Texture2D> MakeTexture2D(const Txp& txp, const char* debugName)
	{
		auto result = MakeUnique<D3D_Texture2D>(txp);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_Texture2D> MakeTexture2D(ivec2 size, const uint32_t* rgbaBuffer, const char* debugName)
	{
		auto result = MakeUnique<D3D_Texture2D>(size, rgbaBuffer);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_CubeMap> MakeCubeMap(const Txp& txp, const char* debugName)
	{
		auto result = MakeUnique<D3D_CubeMap>(txp);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_CubeMap> MakeCubeMap(const LightMapIBL& lightMap, const char* debugName)
	{
		auto result = MakeUnique<D3D_CubeMap>(lightMap);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_IndexBuffer> MakeIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, const char* debugName)
	{
		auto result = MakeUnique<D3D_StaticIndexBuffer>(dataSize, data, indexFormat);
		SetDebugName(*result, debugName);
		return result;
	}

	UniquePtr<GPU_VertexBuffer> MakeVertexBuffer(size_t dataSize, const void* data, size_t stride, const char* debugName)
	{
		auto result = MakeUnique<D3D_StaticVertexBuffer>(dataSize, data, stride);
		SetDebugName(*result, debugName);
		return result;
	}
}
