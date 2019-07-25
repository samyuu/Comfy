#pragma once
#include "Graphics/Auth2D/AetMgr.h"
#include <functional>

namespace Editor
{
	using namespace Auth2D;

	enum BoxNode
	{
		BoxNode_TL = 0,
		BoxNode_TR = 1,
		BoxNode_BL = 2,
		BoxNode_BR = 3,
	};

	struct Box
	{
		Box();
		Box(const Properties& properties, const vec2& dimensions);

		vec2 TL, TR, BL, BR;
		
		vec2 Top() const;
		vec2 Right() const;
		vec2 Bottom() const;
		vec2 Left() const;

		bool Contains(const vec2& point) const;
	};

	class BoxTransformControl
	{
	public:
		void Draw(Properties* properties, vec2 dimensions, const std::function<void(vec2&)>& worldToScreenSpace, const std::function<void(vec2&)>& screenToWorldSpace, float zoom);

	private:
		std::function<void(vec2&)> worldToScreenSpace;
		std::function<void(vec2&)> screenToWorldSpace;
	};
}