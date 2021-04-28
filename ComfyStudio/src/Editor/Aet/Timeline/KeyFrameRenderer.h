#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Graphics/Auth2D/Aet/AetSet.h"

namespace Comfy::Studio::Editor
{
	class AetTimeline;

	class KeyFrameRenderer
	{
	public:
		KeyFrameRenderer();
		~KeyFrameRenderer() = default;

		void DrawContent(const AetTimeline& timeline, const Graphics::Aet::Composition* workingComp);
		vec2 GetCenteredTimelineRowScreenPosition(const AetTimeline& timeline, frame_t frame, i32 row);

	private:
		enum class KeyFrameType { Single, First, Last, InBetween };
		enum class KeyFramePart { Border, FillFull, FillLeft, FillRight, SquareBorder, Square };

	private:
		void CreateKeyFrameTexture();

		void DrawKeyFrameConnection(ImDrawList* drawList, const vec2& start, const vec2& end, bool active) const;
		void DrawKeyFramePart(ImDrawList* drawList, vec2 position, KeyFramePart type, ImU32 color) const;
		void DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, KeyFrameType type, f32 opacity = 1.0f) const;

		static KeyFrameType GetKeyFrameType(const Graphics::Aet::KeyFrame& keyFrame, const Graphics::Aet::Property1D& property);
		static f32 GetKeyFrameOpacity(const Graphics::Aet::KeyFrame& keyFrame, bool opactiyKeyFrames);

	private:
		static constexpr f32 keyFrameSize = 5.5f;
		std::unique_ptr<Graphics::Tex> keyFrameTexture = nullptr;
	};
}
