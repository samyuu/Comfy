#pragma once
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Render::Detail
{
	using TransparencyBitFlags = u8;
	enum TransparencyBitFlags_Enum : TransparencyBitFlags
	{
		TransparencyBitFlags_None = 0,
		TransparencyBitFlags_MaterialForceOpaque = (1 << 0),
		TransparencyBitFlags_MaterialPunchThrough = (1 << 1),
		TransparencyBitFlags_MaterialAlphaMaterial = (1 << 2),
		TransparencyBitFlags_MaterialAlphaTexture = (1 << 3),
		TransparencyBitFlags_SubMeshTransparent = (1 << 4),
		TransparencyBitFlags_Count = (1 << 5),
	};

	using TransparencyTypeFlags = u8;
	enum TransparencyTypeFlags_Enum : TransparencyTypeFlags
	{
		TransparencyTypeFlags_Opaque = 0,
		TransparencyTypeFlags_Transparent = (1 << 0),
		TransparencyTypeFlags_PunchThrough = (1 << 1),
		TransparencyTypeFlags_PunchThroughShadow = (1 << 2),
	};

	// NOTE: Map each valid TransparencyBitFlags combination to the corresponding TransparencyTypeFlags
	constexpr std::array<TransparencyTypeFlags, TransparencyBitFlags_Count> TransparencyBitFlagsToTypeFlagsLookupTable =
	{
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_Transparent,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Transparent,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_Transparent,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_Transparent,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_Transparent,
		TransparencyTypeFlags_PunchThrough,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_Opaque,
		TransparencyTypeFlags_Transparent | TransparencyTypeFlags_PunchThroughShadow,
		TransparencyTypeFlags_PunchThrough | TransparencyTypeFlags_PunchThroughShadow,
	};

	constexpr TransparencyBitFlags GetTransparencyBitFlags(const Graphics::Mesh& mesh, const Graphics::SubMesh& subMesh, const Graphics::Material& material)
	{
		TransparencyBitFlags flags = TransparencyBitFlags_None;

		if (material.BlendFlags.ForceOpaque)
			flags |= TransparencyBitFlags_MaterialForceOpaque;
		if (material.BlendFlags.PunchThrough)
			flags |= TransparencyBitFlags_MaterialPunchThrough;
		if (material.BlendFlags.AlphaMaterial)
			flags |= TransparencyBitFlags_MaterialAlphaMaterial;
		if (material.BlendFlags.AlphaTexture)
			flags |= TransparencyBitFlags_MaterialAlphaTexture;
		if (subMesh.Flags.Transparent)
			flags |= TransparencyBitFlags_SubMeshTransparent;

		return flags;
	};

	constexpr bool IsMeshTransparent(const Graphics::Mesh& mesh, const Graphics::SubMesh& subMesh, const Graphics::Material& material)
	{
		return (TransparencyBitFlagsToTypeFlagsLookupTable[GetTransparencyBitFlags(mesh, subMesh, material)] & TransparencyTypeFlags_Transparent) != 0;
	}

	constexpr bool IsMeshPunchThrough(const Graphics::Mesh& mesh, const Graphics::SubMesh& subMesh, const Graphics::Material& material)
	{
		return (TransparencyBitFlagsToTypeFlagsLookupTable[GetTransparencyBitFlags(mesh, subMesh, material)] & TransparencyTypeFlags_PunchThrough) != 0;
	}

	constexpr bool IsMeshShadowPunchThrough(const Graphics::Mesh& mesh, const Graphics::SubMesh& subMesh, const Graphics::Material& material)
	{
		return (TransparencyBitFlagsToTypeFlagsLookupTable[GetTransparencyBitFlags(mesh, subMesh, material)] & TransparencyTypeFlags_PunchThroughShadow) != 0;
	}
}
