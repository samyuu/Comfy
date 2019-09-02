#pragma once
#include "Graphics/Auth2D/AetMgr.h"
#include <functional>

namespace Editor
{
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

	struct Box
	{
		Box();
		Box(const Graphics::Auth2D::Properties& properties, const vec2& dimensions);

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

	class BoxTransformControl
	{
	public:
		void Draw(Graphics::Auth2D::Properties* properties, vec2 dimensions, const std::function<vec2(vec2)>& worldToScreenSpace, const std::function<vec2(vec2)>& screenToWorldSpace, float zoom);

	private:
		enum GrabMode
		{
			None, Move, Scale, Rotate
		};

		//std::function<void(vec2&)> worldToScreenSpace;
		//std::function<void(vec2&)> screenToWorldSpace;

		vec2 scaleNodeWorldPositionOnMouseDown;
		vec2 mouseWorldPositionOnMouseDown;
		Box screenSpaceBox, worldSpaceBox;
		Graphics::Auth2D::Properties propertiesOnMouseDown;

		GrabMode mode = GrabMode::None;
		int scalingNode, hoveringNode;

		//void MoveBoxPosition(Box& box, vec2 position);
		void MoveBoxCorner(Box& box, vec2 position, float rotation);

		void DragPositionTooltip(const vec2& position);
		void DragScaleTooltip(const vec2& scale, const vec2& dimensions);
	};
}