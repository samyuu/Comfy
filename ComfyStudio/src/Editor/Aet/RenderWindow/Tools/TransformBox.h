#pragma once
#include "Graphics/Auth2D/AetMgr.h"

namespace Comfy::Editor
{
	inline void RotatePointSinCos(vec2& point, float sin, float cos)
	{
		point = vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
	}

	enum BoxNode : int32_t
	{
		BoxNode_Invalid = -1,

		BoxNode_TL = 0,
		BoxNode_TR = 1,
		BoxNode_BL = 2,
		BoxNode_BR = 3,

		BoxNode_Top = 4,
		BoxNode_Right = 5,
		BoxNode_Bottom = 6,
		BoxNode_Left = 7,

		BoxNode_Count,
	};

	struct TransformBox
	{
		// NOTE: Radius of the drawn node
		static constexpr float NodeRadius = 2.0f;
		// NOTE: Slightly larger hitbox node for better user experience
		static constexpr float NodeHitboxRadius = 4.0f;

		static BoxNode GetOpositeNode(BoxNode node);

		TransformBox();
		TransformBox(const Graphics::Transform2D& transform, const vec2& dimensions);

		union
		{
			struct { vec2 TL, TR, BL, BR; };
			vec2 Corners[4];
		};

		vec2 Top() const;
		vec2 Right() const;
		vec2 Bottom() const;
		vec2 Left() const;
		vec2 Center() const;
		float Rotation() const;

		vec2 GetNodePosition(BoxNode node) const;
		Graphics::Transform2D GetTransform(vec2 dimensions, vec2 origin, float rotation, float opacity) const;

		bool Contains(const vec2& point) const;
	};
}