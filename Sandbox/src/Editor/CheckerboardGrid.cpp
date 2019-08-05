#include "CheckerboardGrid.h"

namespace Editor
{
	void CheckerboardGrid::Render(Auth2D::Renderer2D* renderer) const
	{
		renderer->Draw(
			vec2(0.0f),
			Size,
			Color);

		renderer->DrawCheckerboardRectangle(
			vec2(0.0f),
			vec2(1.0f),
			vec2(0.0f),
			0.0f,
			Size,
			ColorAlt,
			renderer->GetCamera()->Zoom * GridSize);
	}
}