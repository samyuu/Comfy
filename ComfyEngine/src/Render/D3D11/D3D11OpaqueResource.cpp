#include "D3D11OpaqueResource.h"
#include "D3D11Buffer.h"
#include "D3D11Texture.h"

namespace Comfy::Render
{
	using namespace Graphics;

	namespace
	{
		// NOTE: Should only be used if the resource type doesn't natively support reuploading new data
		inline void InternalGPUResourceNaiveReuploadIfRequest(const InternallyManagedGPUResource& resource)
		{
			if (resource.RequestReupload)
			{
				resource.Resource = nullptr;
				resource.RequestReupload = false;
			}
		}
	}

	D3D11IndexBuffer* GetD3D11IndexBuffer(D3D11& d3d11, const SubMesh& subMesh)
	{
		InternalGPUResourceNaiveReuploadIfRequest(subMesh.GPU_IndexBuffer);
		if (subMesh.GPU_IndexBuffer.Resource == nullptr)
		{
			subMesh.GPU_IndexBuffer.Resource = std::make_unique<D3D11IndexBuffer>(d3d11, subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat(), D3D11_USAGE_IMMUTABLE);
			D3D11_SetObjectDebugName(static_cast<D3D11IndexBuffer*>(subMesh.GPU_IndexBuffer.Resource.get())->Buffer.Get(),
				"SubMesh IndexBuffer: %s, %s",
				IndexOr(static_cast<size_t>(subMesh.GetIndexFormat()), IndexFormatNames, "Unknown"),
				IndexOr(static_cast<size_t>(subMesh.Primitive), PrimitiveTypeNames, "Unknown"));
		}

		return static_cast<D3D11IndexBuffer*>(subMesh.GPU_IndexBuffer.Resource.get());
	}

	D3D11IndexBuffer* GetD3D11IndexBuffer(D3D11& d3d11, const SubMesh* subMesh)
	{
		return (subMesh != nullptr) ? GetD3D11IndexBuffer(d3d11, *subMesh) : nullptr;
	}

	D3D11VertexBuffer* GetD3D11VertexBuffer(D3D11& d3d11, const Mesh& mesh, VertexAttribute attribute)
	{
		const auto attributeFlag = static_cast<VertexAttributeFlags>(1 << attribute);

		if (!(mesh.AttributeFlags & attributeFlag))
			return nullptr;

		auto checkAttribute = [&](auto& sourceVector, const char* debugName) -> D3D11VertexBuffer*
		{
			auto& gpuVertexBuffer = mesh.GPU_VertexBuffers[attribute];

			InternalGPUResourceNaiveReuploadIfRequest(gpuVertexBuffer);
			if (gpuVertexBuffer.Resource == nullptr)
			{
				using T = decltype(sourceVector[0]);
				gpuVertexBuffer.Resource = std::make_unique<D3D11VertexBuffer>(d3d11, sizeof(T) * sourceVector.size(), sourceVector.data(), sizeof(T), D3D11_USAGE_IMMUTABLE);
				D3D11_SetObjectDebugName(static_cast<D3D11VertexBuffer*>(gpuVertexBuffer.Resource.get())->Buffer.Get(),
					"Mesh VertexBuffer: %s", debugName);
			}

			return static_cast<D3D11VertexBuffer*>(gpuVertexBuffer.Resource.get());
		};

		switch (attribute)
		{
		case VertexAttribute_Position:
			return checkAttribute(mesh.VertexData.Positions, "Positions");
		case VertexAttribute_Normal:
			return checkAttribute(mesh.VertexData.Normals, "Normals");
		case VertexAttribute_Tangent:
			return checkAttribute(mesh.VertexData.Tangents, "Tangents");
		case VertexAttribute_TextureCoordinate0:
			return checkAttribute(mesh.VertexData.TextureCoordinates[0], "TextureCoordinates[0]");
		case VertexAttribute_TextureCoordinate1:
			return checkAttribute(mesh.VertexData.TextureCoordinates[1], "TextureCoordinates[1]");
		case VertexAttribute_TextureCoordinate2:
			return checkAttribute(mesh.VertexData.TextureCoordinates[2], "TextureCoordinates[2]");
		case VertexAttribute_TextureCoordinate3:
			return checkAttribute(mesh.VertexData.TextureCoordinates[3], "TextureCoordinates[3]");
		case VertexAttribute_Color0:
			return checkAttribute(mesh.VertexData.Colors[0], "Colors[0]");
		case VertexAttribute_Color1:
			return checkAttribute(mesh.VertexData.Colors[1], "Colors[1]");
		case VertexAttribute_BoneWeight:
			return checkAttribute(mesh.VertexData.BoneWeights, "BoneWeights");
		case VertexAttribute_BoneIndex:
			return checkAttribute(mesh.VertexData.BoneIndices, "BoneIndices");
		}

		return nullptr;
	}

	D3D11VertexBuffer* GetD3D11VertexBuffer(D3D11& d3d11, const Mesh* mesh, VertexAttribute attribute)
	{
		return (mesh != nullptr) ? GetD3D11VertexBuffer(d3d11, *mesh, attribute) : nullptr;
	}

	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const Tex& tex)
	{
		if (tex.GetSignature() == TxpSig::Texture2D)
		{
			if (tex.GPU_Texture2D.RequestReupload)
			{
				if (tex.GPU_Texture2D.Resource != nullptr)
				{
					if (auto* texture2D = static_cast<D3D11Texture2DAndView*>(tex.GPU_Texture2D.Resource.get()); texture2D->GetIsDynamic())
						texture2D->UploadDataIfDynamic(d3d11, tex);
					else
						tex.GPU_Texture2D.Resource = nullptr;
				}

				tex.GPU_Texture2D.RequestReupload = false;
			}

			if (tex.GPU_Texture2D.Resource == nullptr)
			{
				tex.GPU_Texture2D.Resource = std::make_unique<D3D11Texture2DAndView>(d3d11, tex, tex.GPU_Texture2D.DynamicResource ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE);
				D3D11_SetObjectDebugName(static_cast<D3D11Texture2DAndView*>(tex.GPU_Texture2D.Resource.get())->Texture.Get(),
					"Texture2D: %.*s", static_cast<int>(tex.GetName().size()), tex.GetName().data());
			}

			return static_cast<D3D11Texture2DAndView*>(tex.GPU_Texture2D.Resource.get());
		}
		else if (tex.GetSignature() == TxpSig::CubeMap)
		{
			InternalGPUResourceNaiveReuploadIfRequest(tex.GPU_CubeMap);
			if (tex.GPU_CubeMap.Resource == nullptr)
			{
				tex.GPU_CubeMap.Resource = std::make_unique<D3D11Texture2DAndView>(d3d11, tex, D3D11_USAGE_IMMUTABLE);
				D3D11_SetObjectDebugName(static_cast<D3D11Texture2DAndView*>(tex.GPU_CubeMap.Resource.get())->Texture.Get(),
					"CubeMap: %.*s", static_cast<int>(tex.GetName().size()), tex.GetName().data());
			}

			return static_cast<D3D11Texture2DAndView*>(tex.GPU_CubeMap.Resource.get());
		}
		else
		{
			assert(false);
			return nullptr;
		}
	}

	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const Tex* tex)
	{
		return (tex != nullptr) ? GetD3D11Texture2D(d3d11, *tex) : nullptr;
	}

	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const LightMapIBL& lightMap)
	{
		InternalGPUResourceNaiveReuploadIfRequest(lightMap.GPU_CubeMap);
		if (lightMap.GPU_CubeMap.Resource == nullptr)
		{
			lightMap.GPU_CubeMap.Resource = std::make_unique<D3D11Texture2DAndView>(d3d11, lightMap);
			D3D11_SetObjectDebugName(static_cast<D3D11Texture2DAndView*>(lightMap.GPU_CubeMap.Resource.get())->Texture.Get(),
				"LightMap IBL: (%dx%d)", lightMap.Size.x, lightMap.Size.y);
		}

		return static_cast<D3D11Texture2DAndView*>(lightMap.GPU_CubeMap.Resource.get());
	}

	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const LightMapIBL* lightMap)
	{
		return (lightMap != nullptr) ? GetD3D11Texture2D(d3d11, *lightMap) : nullptr;
	}

	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::Tex& tex)
	{
		D3D11Texture2DAndView* texture2D = GetD3D11Texture2D(d3d11, tex);
		return (texture2D != nullptr) ? texture2D->TextureView.Get() : nullptr;
	}

	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::Tex* tex)
	{
		D3D11Texture2DAndView* texture2D = GetD3D11Texture2D(d3d11, tex);
		return (texture2D != nullptr) ? texture2D->TextureView.Get() : nullptr;
	}

	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::LightMapIBL& lightMap)
	{
		D3D11Texture2DAndView* texture2D = GetD3D11Texture2D(d3d11, lightMap);
		return (texture2D != nullptr) ? texture2D->TextureView.Get() : nullptr;
	}

	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::LightMapIBL* lightMap)
	{
		D3D11Texture2DAndView* texture2D = GetD3D11Texture2D(d3d11, lightMap);
		return (texture2D != nullptr) ? texture2D->TextureView.Get() : nullptr;
	}
}
