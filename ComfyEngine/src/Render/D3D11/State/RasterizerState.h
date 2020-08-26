#pragma once
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render::D3D11
{
	constexpr D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToD3DTopology(Graphics::PrimitiveType primitive)
	{
		switch (primitive)
		{
		case Graphics::PrimitiveType::Points:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

		case Graphics::PrimitiveType::Lines:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		case Graphics::PrimitiveType::LineStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

		case Graphics::PrimitiveType::LineLoop:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

		case Graphics::PrimitiveType::Triangles:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case Graphics::PrimitiveType::TriangleStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		case Graphics::PrimitiveType::TriangleFan:
		case Graphics::PrimitiveType::Quads:
		case Graphics::PrimitiveType::QuadStrip:
		case Graphics::PrimitiveType::Polygon:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		assert(false);
		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}

	class RasterizerState final : IGraphicsResource
	{
	public:
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled = false);
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName);
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName);
		~RasterizerState() = default;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11RasterizerState* GetRasterizerState();

	private:
		D3D11_RASTERIZER_DESC rasterizerDescription;
		ComPtr<ID3D11RasterizerState> rasterizerState;
	};
}
