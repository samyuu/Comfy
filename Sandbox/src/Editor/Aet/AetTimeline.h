#pragma once
#include "Selection.h"
#include "Editor/FrameTimeline.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;
	class AetEditor;

	class AetTimeline : public FrameTimeline
	{
	public:
		AetTimeline();
		~AetTimeline();

		void SetActive(Aet* parent, AetItemTypePtr value);
		bool GetIsPlayback() const override;

	private:
		Aet* aet = nullptr;
		AetItemTypePtr active;

		const float keyFrameSize = 6.0f;
		float rowHeight;
		bool isPlayback = false;
		bool loopPlayback = true;

		float GetTimelineSize() const override;

		enum class KeyFrameType { Single, DoubleX, DoubleY };
		void DrawTimelineContentKeyFrameDoubleX(const vec2& position) const;
		void DrawTimelineContentKeyFrameDoubleY(const vec2& position) const;
		void DrawTimelineContentKeyFrame(const vec2& position, KeyFrameType type) const;

		void DrawTimelineContentNone();
		void DrawTimelineContentKeyFrames();

		void OnDrawTimelineHeaderWidgets() override;
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;
		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;
		void OnDrawTimlineBackground() override;
		void OnUpdate() override;
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

		void UpdateInputCursorClick();

	private:
		enum class PropertyType
		{
			Origin, Position, Rotation, Scale, Opacity, Count
		};

		std::array<const char*, static_cast<size_t>(PropertyType::Count)> timelinePropertyNames =
		{
			"Origin",
			"Position",
			"Rotation",
			"Scale",
			"Opacity",
		};
	};
}