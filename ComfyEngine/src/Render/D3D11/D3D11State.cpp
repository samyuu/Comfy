#include "D3D11State.h"

namespace Comfy::Render
{
	using namespace Graphics;

	namespace
	{
		constexpr UINT SampleMask = 0xFFFFFFFF;

		constexpr std::array<D3D11_BLEND, 4> GetAetBlendParameters(AetBlendMode blendMode)
		{
			switch (blendMode)
			{
			default:
				assert(false);

			case AetBlendMode::Normal:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Add:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Multiply:
				return { D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::LinearDodge:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Overlay:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			}
		}
	}

	D3D11InputLayout::D3D11InputLayout(D3D11& d3d11, const D3D11InputElement* elements, size_t elementCount, const D3D11VertexShader& vertexShader)
		: UsedElementCount(elementCount)
	{
		assert(UsedElementCount <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);

		for (size_t i = 0; i < UsedElementCount; i++)
		{
			const auto& inputElement = elements[i];
			auto& desc = ElementDescs[i];
			desc.SemanticName = inputElement.SemanticName;
			desc.SemanticIndex = inputElement.SemanticIndex;
			desc.Format = inputElement.Format;
			desc.InputSlot = inputElement.InputSlot;
			desc.AlignedByteOffset = inputElement.ByteOffset;
			desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
		}

		d3d11.Device->CreateInputLayout(
			ElementDescs.data(),
			static_cast<UINT>(UsedElementCount),
			vertexShader.BytecodeView.Bytes,
			vertexShader.BytecodeView.Size,
			&InputLayout);
	}

	void D3D11InputLayout::Bind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->IASetInputLayout(InputLayout.Get());
	}

	void D3D11InputLayout::UnBind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->IASetInputLayout(nullptr);
	}

	D3D11BlendState::D3D11BlendState(D3D11& d3d11, AetBlendMode blendMode)
	{
		auto[sourceBlend, destinationBlend, sourceAlphaBlend, destinationAlphaBlend] = GetAetBlendParameters(blendMode);
		BlendStateDesc.AlphaToCoverageEnable = false;
		BlendStateDesc.IndependentBlendEnable = false;
		BlendStateDesc.RenderTarget[0].BlendEnable = true;
		BlendStateDesc.RenderTarget[0].SrcBlend = sourceBlend;
		BlendStateDesc.RenderTarget[0].DestBlend = destinationBlend;
		BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendStateDesc.RenderTarget[0].SrcBlendAlpha = sourceAlphaBlend;
		BlendStateDesc.RenderTarget[0].DestBlendAlpha = destinationAlphaBlend;
		BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		d3d11.Device->CreateBlendState(&BlendStateDesc, &BlendState);
		D3D11_SetObjectDebugName(BlendState.Get(), "AetBlendMode: %s", AetBlendModeNames[static_cast<size_t>(blendMode)]);
	}

	D3D11BlendState::D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend)
		: D3D11BlendState(d3d11, sourceBlend, destinationBlend, D3D11_BLEND_ZERO, D3D11_BLEND_ONE)
	{
	}

	D3D11BlendState::D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlpha, D3D11_BLEND destinationAlpha)
		: D3D11BlendState(d3d11, sourceBlend, destinationBlend, sourceAlpha, destinationAlpha, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD)
	{
	}

	D3D11BlendState::D3D11BlendState(D3D11& d3d11, D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend, 
		D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp, D3D11_COLOR_WRITE_ENABLE writeMask)
	{
		BlendStateDesc.AlphaToCoverageEnable = false;
		BlendStateDesc.IndependentBlendEnable = false;
		BlendStateDesc.RenderTarget[0].BlendEnable = true;
		BlendStateDesc.RenderTarget[0].SrcBlend = sourceBlend;
		BlendStateDesc.RenderTarget[0].DestBlend = destinationBlend;
		BlendStateDesc.RenderTarget[0].BlendOp = blendOp;
		BlendStateDesc.RenderTarget[0].SrcBlendAlpha = sourceAlphaBlend;
		BlendStateDesc.RenderTarget[0].DestBlendAlpha = destinationAlphaBlend;
		BlendStateDesc.RenderTarget[0].BlendOpAlpha = blendAlphaOp;
		BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = writeMask;
		d3d11.Device->CreateBlendState(&BlendStateDesc, &BlendState);
	}

	void D3D11BlendState::Bind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->OMSetBlendState(BlendState.Get(), nullptr, SampleMask);
	}

	void D3D11BlendState::UnBind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->OMSetBlendState(nullptr, nullptr, SampleMask);
	}

	D3D11RasterizerState::D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled)
	{
		RasterizerDesc.FillMode = fillMode;
		RasterizerDesc.CullMode = cullMode;
		// NOTE: Default to true because ObjSet files were originally design to be used with OpenGL
		RasterizerDesc.FrontCounterClockwise = true;
		RasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		RasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable = true;
		RasterizerDesc.ScissorEnable = scissorEnabled;
		RasterizerDesc.MultisampleEnable = false;
		RasterizerDesc.AntialiasedLineEnable = false;
		d3d11.Device->CreateRasterizerState(&RasterizerDesc, &RasterizerState);
	}

	D3D11RasterizerState::D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName)
		: D3D11RasterizerState(d3d11, fillMode, cullMode, scissorEnabled)
	{
		D3D11_SetObjectDebugName(RasterizerState.Get(), debugName);
	}

	D3D11RasterizerState::D3D11RasterizerState(D3D11& d3d11, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName)
		: D3D11RasterizerState(d3d11, fillMode, cullMode, false)
	{
		D3D11_SetObjectDebugName(RasterizerState.Get(), debugName);
	}

	void D3D11RasterizerState::Bind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->RSSetState(RasterizerState.Get());
	}

	void D3D11RasterizerState::UnBind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->RSSetState(nullptr);
	}

	D3D11DepthStencilState::D3D11DepthStencilState(D3D11& d3d11, bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask)
	{
		DepthStencilDesc.DepthEnable = depthEnabled;
		DepthStencilDesc.DepthWriteMask = depthWriteMask;
		DepthStencilDesc.DepthFunc = depthEnabled ? D3D11_COMPARISON_LESS_EQUAL : D3D11_COMPARISON_ALWAYS;
		DepthStencilDesc.StencilEnable = false;
		DepthStencilDesc.StencilReadMask = 0xFF;
		DepthStencilDesc.StencilWriteMask = 0xFF;
		DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		DepthStencilDesc.BackFace = DepthStencilDesc.FrontFace;
		d3d11.Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
	}

	D3D11DepthStencilState::D3D11DepthStencilState(D3D11& d3d11, bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask, const char* debugName)
	{
		D3D11_SetObjectDebugName(DepthStencilState.Get(), debugName);
	}

	void D3D11DepthStencilState::Bind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->OMSetDepthStencilState(DepthStencilState.Get(), 0);
	}

	void D3D11DepthStencilState::UnBind(D3D11& d3d11)
	{
		d3d11.ImmediateContext->OMSetDepthStencilState(nullptr, 0);
	}

	D3D11OcclusionQuery::D3D11OcclusionQuery(D3D11& d3d11)
	{
		QueryDesc.Query = D3D11_QUERY_OCCLUSION;
		QueryDesc.MiscFlags = 0;
		d3d11.Device->CreateQuery(&QueryDesc, &Query);
	}

	D3D11OcclusionQuery::D3D11OcclusionQuery(D3D11& d3d11, const char* debugName)
		: D3D11OcclusionQuery(d3d11)
	{
		D3D11_SetObjectDebugName(Query.Get(), debugName);
	}

	void D3D11OcclusionQuery::BeginQuery(D3D11& d3d11)
	{
		if (Internal.IsMidQuery)
			return;

		Internal.IsFirstQuery = false;

		d3d11.ImmediateContext->Begin(Query.Get());
		Internal.IsMidQuery = true;
	}

	void D3D11OcclusionQuery::EndQuery(D3D11& d3d11)
	{
		Internal.IsMidQuery = false;
		d3d11.ImmediateContext->End(Query.Get());
	}

	void D3D11OcclusionQuery::QueryData(D3D11& d3d11)
	{
		if (Internal.IsFirstQuery || Internal.IsMidQuery)
			return;

		Internal.LastCoveredPixels = Internal.CoveredPixels;
		Internal.CoveredPixels.reset();

		const auto loopStartTime = TimeSpan::GetTimeNow();
		size_t iterationsUntilSuccess = 0;

		while (true)
		{
			UINT64 queryData;
			const HRESULT getDataResult = d3d11.ImmediateContext->GetData(Query.Get(), &queryData, sizeof(UINT64), 0);

			if (getDataResult == S_OK)
			{
				Internal.CoveredPixels = queryData;
				break;
			}

			// NOTE: Should hopefully never happen
			const auto elapsedLoopTime = (TimeSpan::GetTimeNow() - loopStartTime);
			if (elapsedLoopTime > Internal.GetDataSafetyTimeout)
			{
				assert(false);
				Internal.LastCoveredPixels.reset();
				return;
			}

			iterationsUntilSuccess++;
		}
	}

	bool D3D11OcclusionQuery::IsFirstQuery() const
	{
		return Internal.IsFirstQuery;
	}

	bool D3D11OcclusionQuery::HasCoveredPixelsReady() const
	{
		return Internal.LastCoveredPixels.has_value();
	}

	u64 D3D11OcclusionQuery::GetCoveredPixels() const
	{
		return Internal.LastCoveredPixels.value_or(0);
	}
}
