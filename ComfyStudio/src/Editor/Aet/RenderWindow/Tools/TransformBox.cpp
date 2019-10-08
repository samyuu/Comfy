#include "TransformBox.h"

namespace Editor
{
	BoxNode TransformBox::GetOpositeNode(BoxNode node)
	{
		switch (node)
		{
		case BoxNode_TL:
			return BoxNode_BR;
		case BoxNode_TR:
			return BoxNode_BL;
		case BoxNode_BL:
			return BoxNode_TR;
		case BoxNode_BR:
			return BoxNode_TL;
		case BoxNode_Top:
			return BoxNode_Bottom;
		case BoxNode_Right:
			return BoxNode_Left;
		case BoxNode_Bottom:
			return BoxNode_Top;
		case BoxNode_Left:
			return BoxNode_Right;
		}

		assert(false);
		return {};
	}

	TransformBox::TransformBox()
	{
	}

	TransformBox::TransformBox(const Graphics::Auth2D::Properties& properties, const vec2& dimensions)
	{
		vec2 size = dimensions * properties.Scale;

		if (properties.Rotation == 0.0f)
		{
			vec2 position = properties.Position - (properties.Origin * properties.Scale);

			TL = position;
			TR = vec2(position.x + size.x, position.y);
			BL = vec2(position.x, position.y + size.y);
			BR = position + size;
		}
		else
		{
			float radians = glm::radians(properties.Rotation);
			float sin = glm::sin(radians), cos = glm::cos(radians);
			vec2 origin = -properties.Origin * properties.Scale;

			TL = properties.Position + vec2(origin.x * cos - origin.y * sin, origin.x * sin + origin.y * cos);
			TR = properties.Position + vec2((origin.x + size.x) * cos - origin.y * sin, (origin.x + size.x) * sin + origin.y * cos);
			BL = properties.Position + vec2(origin.x * cos - (origin.y + size.y) * sin, origin.x * sin + (origin.y + size.y) * cos);
			BR = properties.Position + vec2((origin.x + size.x) * cos - (origin.y + size.y) * sin, (origin.x + size.x) * sin + (origin.y + size.y) * cos);
		}
	}

	vec2 TransformBox::Top() const
	{
		return (TL + TR) / 2.0f;
	}

	vec2 TransformBox::Right() const
	{
		return (TR + BR) / 2.0f;
	}

	vec2 TransformBox::Bottom() const
	{
		return (BL + BR) / 2.0f;
	}

	vec2 TransformBox::Left() const
	{
		return (TL + BL) / 2.0f;
	}

	vec2 TransformBox::Center() const
	{
		return (TL + BR) / 2.0f;
	}

	float TransformBox::Rotation() const
	{
		const vec2 pointA = Center();
		const vec2 pointB = Left();
		return glm::degrees(glm::atan(pointA.y - pointB.y, pointA.x - pointB.x));
	}

	vec2 TransformBox::GetNodePosition(BoxNode node) const
	{
		switch (node)
		{
		case BoxNode_TL: return TL;
		case BoxNode_TR: return TR;
		case BoxNode_BL: return BL;
		case BoxNode_BR: return BR;
		case BoxNode_Top: return Top();
		case BoxNode_Right: return Right();
		case BoxNode_Bottom: return Bottom();
		case BoxNode_Left: return Left();
		}

		assert(false);
		return vec2(-1.0f);
	}

	Graphics::Auth2D::Properties TransformBox::GetProperties(vec2 dimensions, vec2 origin, float rotation, float opacity) const
	{
		vec2 corners[2] = { TL, BR };
		vec2 rotationorigin = TL - origin;

		float radians = glm::radians(-rotation), sin = glm::sin(radians), cos = glm::cos(radians);
		for (vec2& corner : corners)
		{
			corner -= rotationorigin;
			RotatePointSinCos(corner, sin, cos);
			corner += rotationorigin;
		}

		vec2 scale = (corners[1] - corners[0]) / dimensions;
		vec2 originOffset = (origin * scale);
		RotatePointSinCos(originOffset, glm::sin(glm::radians(rotation)), glm::cos(glm::radians(rotation)));

		return
		{
			origin,
			TL + originOffset,
			rotation,
			scale,
			opacity,
		};
	}

	bool TransformBox::Contains(const vec2& point) const
	{
		// BUG: Doesn't handle zero sizes boxes correctly (?)
		vec2 e = TR - TL;
		vec2 f = BL - TL;

		return !(
			((point.x - TL.x) * e.x + (point.y - TL.y) * e.y < 0.0) ||
			((point.x - TR.x) * e.x + (point.y - TR.y) * e.y > 0.0) ||
			((point.x - TL.x) * f.x + (point.y - TL.y) * f.y < 0.0) ||
			((point.x - BL.x) * f.x + (point.y - BL.y) * f.y > 0.0));
	}
}