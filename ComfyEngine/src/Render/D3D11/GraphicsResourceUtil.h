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
			tex.GPU_Texture2D = std::make_unique<Texture2D>(tex);

		return static_cast<Texture2D*>(tex.GPU_Texture2D.get());
	}

	inline CubeMap* GetCubeMap(const Graphics::Tex& tex)
	{
		if (tex.GetSignature() == Graphics::TxpSig::CubeMap && tex.GPU_CubeMap == nullptr)
			tex.GPU_CubeMap = std::make_unique<CubeMap>(tex);

		return static_cast<CubeMap*>(tex.GPU_CubeMap.get());
	}

	inline CubeMap* GetCubeMap(const Graphics::LightMapIBL& lightMap)
	{
		if (lightMap.GPU_CubeMap == nullptr)
			lightMap.GPU_CubeMap = std::make_unique<CubeMap>(lightMap);

		return static_cast<CubeMap*>(lightMap.GPU_CubeMap.get());
	}

	/*
	namespace
	{
		template <typename T>
		void InitializeBufferIfAttribute(Mesh& mesh, VertexAttribute attribute, std::vector<T>& vertexData, const char* objSetName, const char* bufferName)
		{
			auto& vertexBuffer = mesh.GPU_VertexBuffers[attribute];

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
			{
				vertexBuffer = nullptr;
				return;
			}

			const char* debugName = nullptr;

#if COMFY_D3D11_DEBUG_NAMES
			char debugNameBuffer[128];
			sprintf_s(debugNameBuffer, "<%s> %s::%sBuffer", objSetName, mesh.Name.data(), bufferName);
			debugName = debugNameBuffer;
#endif

			vertexBuffer = GPU::MakeVertexBuffer(vertexData.size() * sizeof(T), vertexData.data(), sizeof(T), debugName);
		}

		void Upload()
		{
			for (auto& mesh : Meshes)
			{
				InitializeBufferIfAttribute(mesh, VertexAttribute_Position, mesh.VertexData.Positions, Name.c_str(), "Posititon");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Normal, mesh.VertexData.Normals, Name.c_str(), "Normal");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, Name.c_str(), "Tangent");
				InitializeBufferIfAttribute(mesh, VertexAttribute_0x3, mesh.VertexData.Reserved0x3, Name.c_str(), "Reserved0x3");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], Name.c_str(), "TextureCoordinate[0]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], Name.c_str(), "TextureCoordinate[1]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], Name.c_str(), "TextureCoordinate[2]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], Name.c_str(), "TextureCoordinate[3]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], Name.c_str(), "Color[0]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], Name.c_str(), "Color[1]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, Name.c_str(), "BoneWeight");
				InitializeBufferIfAttribute(mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, Name.c_str(), "BoneIndex");

				u32 subMeshIndex = 0;
				for (auto& subMesh : mesh.SubMeshes)
				{
					const char* debugName = nullptr;

#if COMFY_D3D11_DEBUG_NAMES
					char debugNameBuffer[128];
					sprintf_s(debugNameBuffer, "<%s> %s[%d]::IndexBuffer", Name.c_str(), mesh.Name.data(), subMeshIndex++);
					debugName = debugNameBuffer;
#endif

					subMesh.GPU_IndexBuffer = GPU::MakeIndexBuffer(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat(), debugName);
				}
			}
		}
	}
	*/

	inline IndexBuffer* GetIndexBuffer(const Graphics::SubMesh& subMesh)
	{
		if (subMesh.GPU_IndexBuffer == nullptr)
			subMesh.GPU_IndexBuffer = std::make_unique<StaticIndexBuffer>(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat());

		return static_cast<IndexBuffer*>(subMesh.GPU_IndexBuffer.get());
	}

	inline VertexBuffer* GetVertexBuffer(const Graphics::Mesh& mesh, Graphics::VertexAttribute attribute)
	{
		const auto attributeFlag = static_cast<Graphics::VertexAttributeFlags>(1 << attribute);

		if (!(mesh.AttributeFlags & attributeFlag))
			return nullptr;

		auto checkAttribute = [&](auto& sourceVector) -> VertexBuffer*
		{
			auto& gpuVertexBuffer = mesh.GPU_VertexBuffers[attribute];
			if (gpuVertexBuffer == nullptr)
			{
				using T = decltype(sourceVector[0]);
				gpuVertexBuffer = std::make_unique<StaticVertexBuffer>(sizeof(T) * sourceVector.size(), sourceVector.data(), sizeof(T));
			}

			return static_cast<VertexBuffer*>(gpuVertexBuffer.get());
		};

		switch (attribute)
		{
		case Graphics::VertexAttribute_Position:
			return checkAttribute(mesh.VertexData.Positions);
		case Graphics::VertexAttribute_Normal:
			return checkAttribute(mesh.VertexData.Normals);
		case Graphics::VertexAttribute_Tangent:
			return checkAttribute(mesh.VertexData.Tangents);
		case Graphics::VertexAttribute_0x3:
			return checkAttribute(mesh.VertexData.Reserved0x3);
		case Graphics::VertexAttribute_TextureCoordinate0:
			return checkAttribute(mesh.VertexData.TextureCoordinates[0]);
		case Graphics::VertexAttribute_TextureCoordinate1:
			return checkAttribute(mesh.VertexData.TextureCoordinates[1]);
		case Graphics::VertexAttribute_TextureCoordinate2:
			return checkAttribute(mesh.VertexData.TextureCoordinates[2]);
		case Graphics::VertexAttribute_TextureCoordinate3:
			return checkAttribute(mesh.VertexData.TextureCoordinates[3]);
		case Graphics::VertexAttribute_Color0:
			return checkAttribute(mesh.VertexData.Colors[0]);
		case Graphics::VertexAttribute_Color1:
			return checkAttribute(mesh.VertexData.Colors[1]);
		case Graphics::VertexAttribute_BoneWeight:
			return checkAttribute(mesh.VertexData.BoneWeights);
		case Graphics::VertexAttribute_BoneIndex:
			return checkAttribute(mesh.VertexData.BoneIndices);
		}

		return nullptr;
	}
}
