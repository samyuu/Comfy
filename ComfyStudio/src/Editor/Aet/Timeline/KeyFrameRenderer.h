#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Graphics/Auth2D/AetSet.h"
#include "Graphics/Direct3D/D3D_Texture.h"

namespace Editor
{
	class AetTimeline;

	class KeyFrameRenderer
	{
	public:
		KeyFrameRenderer();
		~KeyFrameRenderer();

		void Initialize();

		void DrawContent(const AetTimeline* timeline, const Graphics::AetComposition* workingComp);
		vec2 GetCenteredTimelineRowScreenPosition(const AetTimeline* timeline, frame_t frame, int row);

	private:
		static constexpr float keyFrameSize = 5.5f;

		static constexpr ivec2 keyFrameTextureSize = ivec2(22, 22);
		static const uint32_t keyFrameTexturePixels[keyFrameTextureSize.x * keyFrameTextureSize.y];

		UniquePtr<Graphics::D3D_Texture2D> keyFrameTexture = nullptr;

		enum class KeyFrameType
		{
			Single, First, Last, InBetween
		};

		enum class KeyFramePart 
		{ 
			Border, FillFull, FillLeft, FillRight, SquareBorder, Square 
		};

		void CreateKeyFrameTexture();
		
		void DrawKeyFrameConnection(ImDrawList* drawList, const vec2& start, const vec2& end, bool active) const;
		void DrawKeyFramePart(ImDrawList* drawList, vec2 position, KeyFramePart type, ImU32 color) const;
		void DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, KeyFrameType type, float opacity = 1.0f) const;

		static KeyFrameType GetKeyFrameType(const Graphics::AetKeyFrame& keyFrame, const Graphics::KeyFrameCollection& keyFrames);
		static float GetKeyFrameOpacity(const Graphics::AetKeyFrame& keyFrame, bool opactiyKeyFrames);
	};
}
