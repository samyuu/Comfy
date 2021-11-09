#include "ImageGridView.h"

namespace ImGui
{
	namespace
	{
		inline ImRect Shrink(ImRect rect, vec2 shrinkBy)
		{
			rect.Expand(-shrinkBy);
			return rect;
		}
	}

	void ImageGridView::Begin(vec2 childSize)
	{
		// NOTE: Horizontal scrolling should only be active if the size of the window is smaller than that of a single item
		BeginChild("ImageGridView::Child", availableSize, false, ImGuiWindowFlags_HorizontalScrollbar);

		const auto availableContentRegion = GetContentRegionAvail();
		availableSize.x = (childSize.x <= 0.0f) ? availableContentRegion.x : childSize.x;
		availableSize.y = (childSize.y <= 0.0f) ? availableContentRegion.y : childSize.y;

		availableScreenRegion.Min = GetCursorScreenPos();
		availableScreenRegion.Max = availableScreenRegion.Min + availableSize;

		if (const auto itemsPerRow = glm::floor(availableSize.x / imageBoundingBoxSize.x); itemsPerRow < 1.0f)
		{
			perItemSpacing = 0.0f;
			firstItemOffset = 0.0f;

			SetCursorPosY(GetCursorPosY() - (imageBoundingBoxSize.y + textBoundingBoxSize.y));
		}
		else
		{
			perItemSpacing = glm::floor(Comfy::Max(0.0f, (availableSize.x - (itemsPerRow * imageBoundingBoxSize.x)) / itemsPerRow));
			firstItemOffset = glm::floor(perItemSpacing * 0.5f);

			SetCursorPosX(GetCursorPosX() + firstItemOffset - perItemSpacing);
		}
	}

	void ImageGridView::Add(std::string_view name, Comfy::ComfyTextureID image, vec2 imageDimensions, bool flipUV)
	{
		auto cursorPos = GetCursorScreenPos();
		cursorPos.x += perItemSpacing;

		if (cursorPos.x + imageBoundingBoxSize.x > availableScreenRegion.Max.x)
		{
			cursorPos.x = availableScreenRegion.Min.x + firstItemOffset;
			cursorPos.y += (imageBoundingBoxSize.y + textBoundingBoxSize.y);
		}

		const auto imageBoundingBox = ImRect(cursorPos, cursorPos + imageBoundingBoxSize);
		const auto textBoundingBox = ImRect(imageBoundingBox.GetBL(), imageBoundingBox.GetBL() + textBoundingBoxSize);
		const auto totalBoundingBox = ImRect(imageBoundingBox.GetTL(), textBoundingBox.GetBR());

		const auto imageRegion = Shrink(imageBoundingBox, GetStyle().FramePadding);

		constexpr auto unwrappedCubeMapAspectRatio = vec2((4.0f / 3.0f), 1.0f);
		auto adjustedImageBoundingBox = FitFixedAspectRatioImage(imageRegion, image.Data.IsCubeMap ? (imageDimensions * unwrappedCubeMapAspectRatio) : imageDimensions);

		auto drawList = GetWindowDrawList();
		const auto uv = [&]() { return image.Data.IsCubeMap || flipUV ? std::array { UV0, UV1 } : std::array { UV0_R, UV1_R }; }();

		const auto id = GetID(StringViewStart(name), StringViewEnd(name));

		bool isHovered, isHeld;
		bool clicked = ButtonBehavior(totalBoundingBox, id, &isHovered, &isHeld, ImGuiButtonFlags_None);

		ItemAdd(totalBoundingBox, 0);
		if (isHovered || isHeld)
			drawList->AddRectFilled(totalBoundingBox.GetTL(), totalBoundingBox.GetBR(), GetColorU32(isHeld ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));

		if (IsItemVisible())
		{
			checkerboard.AddToDrawList(drawList, adjustedImageBoundingBox);
			drawList->AddImage(image, adjustedImageBoundingBox.GetTL(), adjustedImageBoundingBox.GetBR(), uv[0], uv[1]);

			float fontScale = 1.0f;
			vec2 textPosition = textBoundingBox.GetTL();
			{
				auto font = GetFont();
				auto textSize = font->CalcTextSizeA(font->FontSize, std::numeric_limits<float>::max(), -1.0f, StringViewStart(name), StringViewEnd(name));

				const float targetTextWidth = textBoundingBoxSize.x - GetStyle().FramePadding.x;
				if (textSize.x > targetTextWidth)
				{
					fontScale = (targetTextWidth / textSize.x);
					textSize *= fontScale;

					textPosition.y += font->FontSize - (font->FontSize * fontScale);
				}

				textPosition.x += (textBoundingBoxSize.x * 0.5f) - (textSize.x * 0.5f);
			}

			const auto textClipRect = ImVec4(textBoundingBox.Min.x, textBoundingBox.Min.y, textBoundingBox.Max.x, textBoundingBox.Max.y);
			drawList->AddText(nullptr, GetFontSize() * fontScale, textPosition, GetColorU32(ImGuiCol_Text), StringViewStart(name), StringViewEnd(name), 0.0f, &textClipRect);
		}

		SetCursorScreenPos(cursorPos + vec2(imageBoundingBoxSize.x, 0.0));
	}

	void ImageGridView::End()
	{
		EndChild();
		availableSize = {};
	}
}
