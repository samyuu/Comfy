#pragma once
#include "Types.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;
	class AetTimeline;

	class KeyFrameRenderer
	{
	public:
		KeyFrameRenderer();
		~KeyFrameRenderer();

		void DrawKeyFrames(const AetTimeline* timeline, const KeyFrameProperties& keyFrames);

	private:
		const float keyFrameSize = 5.5f;

		void DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position) const;
	};
}
