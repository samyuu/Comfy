#pragma once
#include "D3D11.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render
{
	constexpr DXGI_FORMAT IndexFormatToDXGIFormat(Graphics::IndexFormat indexFormat)
	{
		switch (indexFormat)
		{
		case Graphics::IndexFormat::U8:
			return DXGI_FORMAT_R8_UINT;
		case Graphics::IndexFormat::U16:
			return DXGI_FORMAT_R16_UINT;
		case Graphics::IndexFormat::U32:
			return DXGI_FORMAT_R32_UINT;
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	constexpr size_t GetIndexFormatSize(Graphics::IndexFormat indexFormat)
	{
		switch (indexFormat)
		{
		case Graphics::IndexFormat::U8:
			return sizeof(u8);
		case Graphics::IndexFormat::U16:
			return sizeof(u16);
		case Graphics::IndexFormat::U32:
			return sizeof(u32);
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	constexpr DXGI_FORMAT GetDXGIFormat(Graphics::TextureFormat format)
	{
		using namespace Graphics;
		switch (format)
		{
		case TextureFormat::A8:			return DXGI_FORMAT_R8_UINT;
		case TextureFormat::RGB8:		return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGB5:		return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::RGB5_A1:	return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::RGBA4:		return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::DXT1:		return DXGI_FORMAT_BC1_UNORM;
		case TextureFormat::DXT1a:		return DXGI_FORMAT_BC1_UNORM_SRGB;
		case TextureFormat::DXT3:		return DXGI_FORMAT_BC2_UNORM;
		case TextureFormat::DXT5:		return DXGI_FORMAT_BC3_UNORM;
		case TextureFormat::RGTC1:		return DXGI_FORMAT_BC4_UNORM;
		case TextureFormat::RGTC2:		return DXGI_FORMAT_BC5_UNORM;
		case TextureFormat::L8:			return DXGI_FORMAT_A8_UNORM;
		case TextureFormat::L8A8:		return DXGI_FORMAT_A8P8;
		case TextureFormat::Unknown:	return DXGI_FORMAT_UNKNOWN;
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	constexpr DXGI_FORMAT GetDXGIFormat(Graphics::LightMapFormat format)
	{
		using namespace Graphics;
		switch (format)
		{
		case LightMapFormat::RGBA8_CUBE:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		case LightMapFormat::RGBA16F_CUBE:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case LightMapFormat::RGBA32F_CUBE:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}


	constexpr D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToD3D(Graphics::TextureAddressMode addressMode)
	{
		using namespace Graphics;
		switch (addressMode)
		{
		case TextureAddressMode::ClampBorder:			return D3D11_TEXTURE_ADDRESS_BORDER;
		case TextureAddressMode::ClampEdge:				return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TextureAddressMode::WrapRepeat:			return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureAddressMode::WrapRepeatMirror:		return D3D11_TEXTURE_ADDRESS_MIRROR;
		case TextureAddressMode::WrapRepeatMirrorOnce:	return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		}

		assert(false);
		return D3D11_TEXTURE_ADDRESS_MODE {};
	}

	constexpr D3D11_FILTER TextureFilterToD3D(Graphics::TextureFilter filter)
	{
		using namespace Graphics;
		switch (filter)
		{
		case TextureFilter::Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case TextureFilter::Point:	return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

		assert(false);
		return D3D11_FILTER {};
	}

	constexpr D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToD3DTopology(Graphics::PrimitiveType primitive)
	{
		using namespace Graphics;
		switch (primitive)
		{
		case PrimitiveType::Points:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

		case PrimitiveType::Lines:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		case PrimitiveType::LineStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

		case PrimitiveType::LineLoop:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

		case PrimitiveType::Triangles:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case PrimitiveType::TriangleStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		case PrimitiveType::TriangleFan:
		case PrimitiveType::Quads:
		case PrimitiveType::QuadStrip:
		case PrimitiveType::Polygon:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		assert(false);
		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}
