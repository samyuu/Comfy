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
		if (tex.GetSignature() == Graphics::TxpSig::Texture2D && tex.GPU_Texture2D == nullptr)
		{
			tex.GPU_Texture2D = std::make_unique<Texture2D>(tex);
			if (tex.GPU_Texture2D != nullptr)
			{
				D3D11_SetObjectDebugName(static_cast<Texture2D*>(tex.GPU_Texture2D.get())->GetTexture(),
					"Texture2D: %.*s",
					static_cast<int>(tex.GetName().size()),
					tex.GetName().data());
			}
		}

		return static_cast<Texture2D*>(tex.GPU_Texture2D.get());
	}

	inline Texture2D* GetTexture2D(const Graphics::Tex* tex)
	{
		return (tex != nullptr) ? GetTexture2D(*tex) : nullptr;
	}

	inline CubeMap* GetCubeMap(const Graphics::Tex& tex)
	{
		if (tex.GetSignature() == Graphics::TxpSig::CubeMap && tex.GPU_CubeMap == nullptr)
		{
			tex.GPU_CubeMap = std::make_unique<CubeMap>(tex);
			if (tex.GPU_CubeMap != nullptr)
			{
				D3D11_SetObjectDebugName(static_cast<CubeMap*>(tex.GPU_CubeMap.get())->GetTexture(),
					"CubeMap: %.*s",
					static_cast<int>(tex.GetName().size()),
					tex.GetName().data());
			}
		}

		return static_cast<CubeMap*>(tex.GPU_CubeMap.get());
	}

	inline CubeMap* GetCubeMap(const Graphics::LightMapIBL& lightMap)
	{
		if (lightMap.GPU_CubeMap == nullptr)
		{
			lightMap.GPU_CubeMap = std::make_unique<CubeMap>(lightMap);
			if (lightMap.GPU_CubeMap != nullptr)
			{
				D3D11_SetObjectDebugName(static_cast<CubeMap*>(lightMap.GPU_CubeMap.get())->GetTexture(),
					"LightMap IBL: (%dx%d)",
					lightMap.Size.x,
					lightMap.Size.y);
			}
		}

		return static_cast<CubeMap*>(lightMap.GPU_CubeMap.get());
	}

	inline IndexBuffer* GetIndexBuffer(const Graphics::SubMesh& subMesh)
	{
		if (subMesh.GPU_IndexBuffer == nullptr)
		{
			subMesh.GPU_IndexBuffer = std::make_unique<StaticIndexBuffer>(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat());
			if (subMesh.GPU_IndexBuffer != nullptr)
			{
				D3D11_SetObjectDebugName(static_cast<StaticIndexBuffer*>(subMesh.GPU_IndexBuffer.get())->GetBuffer(),
					"SubMesh IndexBuffer: %s, %s",
					IndexOr(static_cast<size_t>(subMesh.GetIndexFormat()), Graphics::IndexFormatNames, "Unknown"),
					IndexOr(static_cast<size_t>(subMesh.Primitive), Graphics::PrimitiveTypeNames, "Unknown"));
			}
		}

		return static_cast<IndexBuffer*>(subMesh.GPU_IndexBuffer.get());
	}

	inline VertexBuffer* GetVertexBuffer(const Graphics::Mesh& mesh, Graphics::VertexAttribute attribute)
	{
		const auto attributeFlag = static_cast<Graphics::VertexAttributeFlags>(1 << attribute);

		if (!(mesh.AttributeFlags & attributeFlag))
			return nullptr;

		auto checkAttribute = [&](auto& sourceVector, const char* debugName) -> VertexBuffer*
		{
			auto& gpuVertexBuffer = mesh.GPU_VertexBuffers[attribute];
			if (gpuVertexBuffer == nullptr)
			{
				using T = decltype(sourceVector[0]);
				gpuVertexBuffer = std::make_unique<StaticVertexBuffer>(sizeof(T) * sourceVector.size(), sourceVector.data(), sizeof(T));

				if (gpuVertexBuffer != nullptr)
				{
					D3D11_SetObjectDebugName(static_cast<StaticVertexBuffer*>(gpuVertexBuffer.get())->GetBuffer(),
						"Mesh VertexBuffer: %s",
						debugName);
				}
			}

			return static_cast<VertexBuffer*>(gpuVertexBuffer.get());
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
