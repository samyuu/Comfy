#include "imgui_extensions.h"
#include "../Rendering/Texture.h"

namespace ImGui
{
	void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		float width = texture->GetWidth() * scale.x;
		float height = texture->GetHeight() * scale.y;

		center.x -= width * .5f;
		center.y -= height * .5f;
		ImVec2 bottomRight(center.x + width, center.y + height);

		drawList->AddImage(texture->GetVoidTexture(), center, bottomRight, uv0, uv1);
	}
}