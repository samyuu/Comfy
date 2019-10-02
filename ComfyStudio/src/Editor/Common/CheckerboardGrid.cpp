#include "CheckerboardGrid.h"

namespace Editor
{
	void CheckerboardGrid::Render(Graphics::Auth2D::Renderer2D* renderer) const
	{
		renderer->Draw(
			Position,
			Size,
			Color);

		renderer->DrawCheckerboardRectangle(
			Position,
			vec2(1.0f),
			vec2(0.0f),
			0.0f,
			Size,
			ColorAlt,
			renderer->GetCamera()->Zoom / GridSize);
	}
}