#pragma once
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Render::Detail
{
	enum class LensFlareGhostType
	{
		TL,
		TR,
		BL,
		BR,
		Count,

		Orange = TL,
		Yellow = TR,
		Pentagon = BL,
		Rainbow = BR,
	};

	struct LensFlareGhostInfo
	{
		float CenterDistance, Scale, Opacity;
		LensFlareGhostType Type;
	};

	constexpr std::array<LensFlareGhostInfo, 16> LensFlareGhostLayout
	{
		LensFlareGhostInfo { -0.17f, 0.70f, 0.60f, LensFlareGhostType::Pentagon },
		LensFlareGhostInfo { -0.10f, 0.40f, 0.70f, LensFlareGhostType::Rainbow },
		LensFlareGhostInfo { +0.06f, 0.35f, 0.80f, LensFlareGhostType::Orange },
		LensFlareGhostInfo { +0.10f, 0.50f, 0.70f, LensFlareGhostType::Yellow },

		LensFlareGhostInfo { +0.14f, 0.40f, 0.80f, LensFlareGhostType::Pentagon },
		LensFlareGhostInfo { +0.04f, 0.30f, 0.70f, LensFlareGhostType::Rainbow },
		LensFlareGhostInfo { -0.13f, 0.60f, 0.60f, LensFlareGhostType::Orange },
		LensFlareGhostInfo { -0.22f, 0.40f, 0.80f, LensFlareGhostType::Yellow },

		LensFlareGhostInfo { -0.45f, 2.50f, 0.40f, LensFlareGhostType::Pentagon },
		LensFlareGhostInfo { -0.80f, 0.80f, 0.50f, LensFlareGhostType::Rainbow },
		LensFlareGhostInfo { +0.20f, 0.50f, 0.80f, LensFlareGhostType::Orange },
		LensFlareGhostInfo { +0.41f, 0.50f, 0.80f, LensFlareGhostType::Yellow },

		LensFlareGhostInfo { -0.70f, 1.30f, 0.80f, LensFlareGhostType::Pentagon },
		LensFlareGhostInfo { -0.30f, 1.50f, 1.00f, LensFlareGhostType::Rainbow },
		LensFlareGhostInfo { +0.35f, 1.00f, 1.00f, LensFlareGhostType::Orange },
		LensFlareGhostInfo { +0.50f, 1.10f, 1.00f, LensFlareGhostType::Yellow },
	};

	constexpr vec2 GetLensFlareGhostPosition(const LensFlareGhostInfo& info, vec2 center, vec2 sunPosition)
	{ 
		return (1.0f - info.CenterDistance) * center + info.CenterDistance * sunPosition;
	}

	class LensFlareMesh : NonCopyable
	{
	public:
		LensFlareMesh() = default;
		~LensFlareMesh() = default;

	public:
		inline const Graphics::Mesh& GetVertexBufferMesh() const
		{
			return underlyingMesh;
		}

		inline const Graphics::SubMesh& GetSunSubMesh() const
		{
			return underlyingMesh.SubMeshes.back();
		}

		inline const Graphics::SubMesh& GetGhostSubMesh(LensFlareGhostType type) const
		{
			const auto typeIndex = static_cast<size_t>(type);
			assert(InBounds(typeIndex, underlyingMesh.SubMeshes) && type != LensFlareGhostType::Count);

			return underlyingMesh.SubMeshes[typeIndex];
		}

	private:
		Graphics::Mesh underlyingMesh = []
		{
			constexpr auto subMeshCount = (static_cast<u32>(LensFlareGhostType::Count) + 1);

			Graphics::Mesh mesh = {};
			mesh.AttributeFlags = (Graphics::VertexAttributeFlags_Position | Graphics::VertexAttributeFlags_TextureCoordinate0);
			mesh.VertexData.Stride = sizeof(vec3) + sizeof(vec2);
			mesh.VertexData.VertexCount = 4 * subMeshCount;
			mesh.VertexData.Positions =
			{
				// NOTE: Center orientated 1x1 flat plane copies
				vec3(-0.5f, +0.5f, 0.0f), vec3(-0.5f, -0.5f, 0.0f), vec3(+0.5f, -0.5f, 0.0f), vec3(+0.5f, +0.5f, 0.0f),
				vec3(-0.5f, +0.5f, 0.0f), vec3(-0.5f, -0.5f, 0.0f), vec3(+0.5f, -0.5f, 0.0f), vec3(+0.5f, +0.5f, 0.0f),
				vec3(-0.5f, +0.5f, 0.0f), vec3(-0.5f, -0.5f, 0.0f), vec3(+0.5f, -0.5f, 0.0f), vec3(+0.5f, +0.5f, 0.0f),
				vec3(-0.5f, +0.5f, 0.0f), vec3(-0.5f, -0.5f, 0.0f), vec3(+0.5f, -0.5f, 0.0f), vec3(+0.5f, +0.5f, 0.0f),
				vec3(-0.5f, +0.5f, 0.0f), vec3(-0.5f, -0.5f, 0.0f), vec3(+0.5f, -0.5f, 0.0f), vec3(+0.5f, +0.5f, 0.0f),
			};
			mesh.VertexData.TextureCoordinates[0] =
			{
				// NOTE: Lens flare ghost top left
				vec2(0.00f, 1.00f), vec2(0.00f, 0.50f), vec2(0.50f, 0.50f), vec2(0.50f, 1.00f),
				// NOTE: Lens flare ghost top right
				vec2(0.50f, 1.00f), vec2(0.50f, 0.50f), vec2(1.00f, 0.50f), vec2(1.00f, 1.00f),
				// NOTE: Lens flare ghost bottom left
				vec2(0.00f, 0.50f), vec2(0.00f, 0.00f), vec2(0.50f, 0.00f), vec2(0.50f, 0.50f),
				// NOTE: Lens flare ghost bottom right
				vec2(0.50f, 0.50f), vec2(0.50f, 0.00f), vec2(1.00f, 0.00f), vec2(1.00f, 0.50f),
				// NOTE: Lens flare sun
				vec2(0.15f, 0.85f), vec2(0.15f, 0.15f), vec2(0.85f, 0.15f), vec2(0.85f, 0.85f),
			};

			for (u16 i = 0; i < subMeshCount; i++)
			{
				auto& subMesh = mesh.SubMeshes.emplace_back();
				subMesh.Primitive = Graphics::PrimitiveType::Triangles;

				const u16 indexOffset = (i * 4);
				auto& indices = subMesh.Indices.emplace<std::vector<u16>>();
				indices.reserve(6);
				indices.push_back(0 + indexOffset);
				indices.push_back(1 + indexOffset);
				indices.push_back(3 + indexOffset);
				indices.push_back(1 + indexOffset);
				indices.push_back(2 + indexOffset);
				indices.push_back(3 + indexOffset);
			}

			return mesh;
		}();
	};
}
