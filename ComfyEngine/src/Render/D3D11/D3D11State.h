#pragma once
#include "Types.h"
#include "D3D11.h"
#include "D3D11Shader.h"
#include "Graphics/GraphicTypes.h"
#include "Time/TimeSpan.h"
#include <optional>

namespace Comfy::Render
{
	// TODO: Use easier more limited custom format enum instead of the DXGI format type
	struct D3D11InputElement
	{
		const char* SemanticName;
		u32 SemanticIndex;
		DXGI_FORMAT Format;
		u32 ByteOffset;
		u32 InputSlot = 0;
	};

	struct D3D11InputLayout : NonCopyable
	{
		D3D11InputLayout(D3D11& d3d11, const D3D11InputElement* elements, size_t elementCount, const D3D11VertexShader& vertexShader);
		~D3D11InputLayout() = default;

		void Bind(D3D11& d3d11);
		void UnBind(D3D11& d3d11);

		const size_t UsedElementCount = {};
		std::array<D3D11_INPUT_ELEMENT_DESC, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> ElementDescs = {};
		ComPtr<ID3D11InputLayout> InputLayout = {};
	};

	struct D3D11BlendState : NonCopyable
	{
		D3D11BlendState(D3D11& d3d11, Graphics::AetBlendMode blendMode);
		D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend);
		D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlpha, D3D11_BLEND destinationAlpha);
		D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend,
			D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp, D3D11_COLOR_WRITE_ENABLE writeMask = D3D11_COLOR_WRITE_ENABLE_ALL);
		~D3D11BlendState() = default;

		void Bind(D3D11& d3d11);
		void UnBind(D3D11& d3d11);

		D3D11_BLEND_DESC BlendStateDesc = {};
		ComPtr<ID3D11BlendState> BlendState = {};
	};

	struct D3D11RasterizerState : NonCopyable
	{
		D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled = false);
		D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName);
		D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName);
		~D3D11RasterizerState() = default;

		void Bind(D3D11& d3d11);
		void UnBind(D3D11& d3d11);

		D3D11_RASTERIZER_DESC RasterizerDesc = {};
		ComPtr<ID3D11RasterizerState> RasterizerState = {};
	};

	struct D3D11DepthStencilState : NonCopyable
	{
		D3D11DepthStencilState(D3D11& d3d11, bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask);
		D3D11DepthStencilState(D3D11& d3d11, bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask, const char* debugName);
		~D3D11DepthStencilState() = default;

		void Bind(D3D11& d3d11);
		void UnBind(D3D11& d3d11);

		D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
		ComPtr<ID3D11DepthStencilState> DepthStencilState = {};
	};

	struct D3D11OcclusionQuery : NonCopyable
	{
		D3D11OcclusionQuery(D3D11& d3d11);
		D3D11OcclusionQuery(D3D11& d3d11, const char* debugName);
		~D3D11OcclusionQuery() = default;

		// NOTE: Call before issuing draw calls
		void BeginQuery(D3D11& d3d11);
		// NOTE: Call after issuing draw calls
		void EndQuery(D3D11& d3d11);

		// NOTE: Call after having begun and then ended the query
		void QueryData(D3D11& d3d11);

		bool IsFirstQuery() const;
		bool HasCoveredPixelsReady() const;
		u64 GetCoveredPixels() const;

		struct InternalData
		{
			// NOTE: In case requersting the data takes too long or worst case never succeeds at all
			static constexpr TimeSpan GetDataSafetyTimeout = TimeSpan::FromMilliseconds(75.0);

			bool IsMidQuery = false;
			bool IsFirstQuery = true;
			std::optional<u64> CoveredPixels = {};
			std::optional<u64> LastCoveredPixels = {};
		} Internal = {};

		D3D11_QUERY_DESC QueryDesc = {};
		ComPtr<ID3D11Query> Query = {};
	};
}
