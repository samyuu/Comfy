#pragma once
#include "Types.h"
#include "IGraphicsResource.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/LightParam/IBLParameters.h"
#include "Texture/Texture.h"
#include "Buffer/IndexBuffer.h"
#include "Buffer/VertexBuffer.h"

namespace Comfy::Render::D3D11
{
	inline Texture2D* GetTexture2D(const Graphics::Tex& tex)
	{
		if (tex.GetSignature() != Graphics::TxpSig::Texture2D)
			return nullptr;

		// TODO:
		if (tex.GPU_Texture2D.RequestReupload) { tex.GPU_Texture2D.Resource = nullptr; }

		if (tex.GPU_Texture2D.Resource == nullptr)
		{
			// TODO: 
			if (tex.GPU_Texture2D.DynamicResource) {}

			tex.GPU_Texture2D.Resource = std::make_unique<Texture2D>(tex);
			D3D11_SetObjectDebugName(static_cast<Texture2D*>(tex.GPU_Texture2D.Resource.get())->GetTexture(),
				"Texture2D: %.*s", static_cast<int>(tex.GetName().size()), tex.GetName().data());
		}

		return static_cast<Texture2D*>(tex.GPU_Texture2D.Resource.get());
	}

	inline Texture2D* GetTexture2D(const Graphics::Tex* tex)
	{
		return (tex != nullptr) ? GetTexture2D(*tex) : nullptr;
	}

	inline CubeMap* GetCubeMap(const Graphics::Tex& tex)
	{
		if (tex.GetSignature() != Graphics::TxpSig::CubeMap)
			return nullptr;

		// TODO:
		if (tex.GPU_CubeMap.RequestReupload) { tex.GPU_CubeMap.Resource = nullptr; }

		if (tex.GPU_CubeMap.Resource == nullptr)
		{
			tex.GPU_CubeMap.Resource = std::make_unique<CubeMap>(tex);
			D3D11_SetObjectDebugName(static_cast<CubeMap*>(tex.GPU_CubeMap.Resource.get())->GetTexture(),
				"CubeMap: %.*s", static_cast<int>(tex.GetName().size()), tex.GetName().data());
		}

		return static_cast<CubeMap*>(tex.GPU_CubeMap.Resource.get());
	}

	inline CubeMap* GetCubeMap(const Graphics::LightMapIBL& lightMap)
	{
		// TODO:
		if (lightMap.GPU_CubeMap.RequestReupload) { lightMap.GPU_CubeMap.Resource = nullptr; }

		if (lightMap.GPU_CubeMap.Resource == nullptr)
		{
			lightMap.GPU_CubeMap.Resource = std::make_unique<CubeMap>(lightMap);
			D3D11_SetObjectDebugName(static_cast<CubeMap*>(lightMap.GPU_CubeMap.Resource.get())->GetTexture(),
				"LightMap IBL: (%dx%d)", lightMap.Size.x, lightMap.Size.y);
		}

		return static_cast<CubeMap*>(lightMap.GPU_CubeMap.Resource.get());
	}

	inline IndexBuffer* GetIndexBuffer(const Graphics::SubMesh& subMesh)
	{
		// TODO:
		if (subMesh.GPU_IndexBuffer.RequestReupload) { subMesh.GPU_IndexBuffer.Resource = nullptr; }

		if (subMesh.GPU_IndexBuffer.Resource == nullptr)
		{
			subMesh.GPU_IndexBuffer.Resource = std::make_unique<StaticIndexBuffer>(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat());
			D3D11_SetObjectDebugName(static_cast<StaticIndexBuffer*>(subMesh.GPU_IndexBuffer.Resource.get())->GetBuffer(),
				"SubMesh IndexBuffer: %s, %s",
				IndexOr(static_cast<size_t>(subMesh.GetIndexFormat()), Graphics::IndexFormatNames, "Unknown"),
				IndexOr(static_cast<size_t>(subMesh.Primitive), Graphics::PrimitiveTypeNames, "Unknown"));
		}

		return static_cast<IndexBuffer*>(subMesh.GPU_IndexBuffer.Resource.get());
	}

	inline VertexBuffer* GetVertexBuffer(const Graphics::Mesh& mesh, Graphics::VertexAttribute attribute)
	{
		const auto attributeFlag = static_cast<Graphics::VertexAttributeFlags>(1 << attribute);

		if (!(mesh.AttributeFlags & attributeFlag))
			return nullptr;

		auto checkAttribute = [&](auto& sourceVector, const char* debugName) -> VertexBuffer*
		{
			auto& gpuVertexBuffer = mesh.GPU_VertexBuffers[attribute];

			// TODO:
			if (gpuVertexBuffer.RequestReupload) { gpuVertexBuffer.Resource = nullptr; }

			if (gpuVertexBuffer.Resource == nullptr)
			{
				using T = decltype(sourceVector[0]);
				gpuVertexBuffer.Resource = std::make_unique<StaticVertexBuffer>(sizeof(T) * sourceVector.size(), sourceVector.data(), sizeof(T));

				D3D11_SetObjectDebugName(static_cast<StaticVertexBuffer*>(gpuVertexBuffer.Resource.get())->GetBuffer(),
					"Mesh VertexBuffer: %s", debugName);
			}

			return static_cast<VertexBuffer*>(gpuVertexBuffer.Resource.get());
		};

		switch (attribute)
		{
		case Graphics::VertexAttribute_Position:
			return checkAttribute(mesh.VertexData.Positions, "Positions");
		case Graphics::VertexAttribute_Normal:
			return checkAttribute(mesh.VertexData.Normals, "Normals");
		case Graphics::VertexAttribute_Tangent:
			return checkAttribute(mesh.VertexData.Tangents, "Tangents");
		case Graphics::VertexAttribute_TextureCoordinate0:
			return checkAttribute(mesh.VertexData.TextureCoordinates[0], "TextureCoordinates[0]");
		case Graphics::VertexAttribute_TextureCoordinate1:
			return checkAttribute(mesh.VertexData.TextureCoordinates[1], "TextureCoordinates[1]");
		case Graphics::VertexAttribute_TextureCoordinate2:
			return checkAttribute(mesh.VertexData.TextureCoordinates[2], "TextureCoordinates[2]");
		case Graphics::VertexAttribute_TextureCoordinate3:
			return checkAttribute(mesh.VertexData.TextureCoordinates[3], "TextureCoordinates[3]");
		case Graphics::VertexAttribute_Color0:
			return checkAttribute(mesh.VertexData.Colors[0], "Colors[0]");
		case Graphics::VertexAttribute_Color1:
			return checkAttribute(mesh.VertexData.Colors[1], "Colors[1]");
		case Graphics::VertexAttribute_BoneWeight:
			return checkAttribute(mesh.VertexData.BoneWeights, "BoneWeights");
		case Graphics::VertexAttribute_BoneIndex:
			return checkAttribute(mesh.VertexData.BoneIndices, "BoneIndices");
		}

		return nullptr;
	}
}
