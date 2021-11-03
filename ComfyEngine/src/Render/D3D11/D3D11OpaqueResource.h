#pragma once
#include "D3D11.h"
#include "Graphics/GPUResource.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/LightParam/IBLParameters.h"

namespace Comfy::Render
{
	struct D3D11IndexBuffer;
	struct D3D11VertexBuffer;
	struct D3D11Texture2DAndView;

	struct D3D11OpaqueResource : public Graphics::OpaqueGPUResource
	{
		virtual ~D3D11OpaqueResource() = default;
	};

	D3D11IndexBuffer* GetD3D11IndexBuffer(D3D11& d3d11, const Graphics::SubMesh& subMesh);
	D3D11IndexBuffer* GetD3D11IndexBuffer(D3D11& d3d11, const Graphics::SubMesh* subMesh);
	D3D11VertexBuffer* GetD3D11VertexBuffer(D3D11& d3d11, const Graphics::Mesh& mesh, Graphics::VertexAttribute attribute);
	D3D11VertexBuffer* GetD3D11VertexBuffer(D3D11& d3d11, const Graphics::Mesh* mesh, Graphics::VertexAttribute attribute);
	
	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const Graphics::Tex& tex);
	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const Graphics::Tex* tex);
	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11, const Graphics::LightMapIBL& lightMap);
	D3D11Texture2DAndView* GetD3D11Texture2D(D3D11& d3d11,const Graphics::LightMapIBL* lightMap);

	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::Tex& tex);
	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::Tex* tex);
	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::LightMapIBL& lightMap);
	ID3D11ShaderResourceView* GetD3D11Texture2DView(D3D11& d3d11, const Graphics::LightMapIBL* lightMap);
}
