#pragma once
#include "Graphics/Auth2D/AetMgr.h"

namespace Editor
{
	inline void RotatePointSinCos(vec2& point, float sin, float cos)
	{
		point = vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
	}

	enum BoxNode
	{
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
		static constexpr float NodeRadius = 4.0f;

		TransformBox();
		TransformBox(const Graphics::Auth2D::Properties& properties, const vec2& dimensions);

		union
		{
			struct { vec2 TL, TR, BL, BR; };
			vec2 Corners[4];
		};

		vec2 Top() const;
		vec2 Right() const;
		vec2 Bottom() const;
		vec2 Left() const;

		vec2 GetNodePosition(int node);
		Graphics::Auth2D::Properties GetProperties(vec2 dimensions, vec2 origin, float rotation, float opacity) const;

		bool Contains(const vec2& point) const;
	};
}