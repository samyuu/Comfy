#pragma once
#include "Selection.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AetInspector
	{
	public:
		AetInspector();
		~AetInspector();

		void Initialize();
		bool DrawGui(AetSet* aetSet, const AetItemTypePtr& selected);

	private:
		void DrawInspectorAetObj(AetSet* aetSet, AetObj* aetObj);
		void DrawInspectorRegionData(AetRegion* spriteEntry);
		void DrawInspectorLayerData(AetLayer* aetLayer);
		void DrawInspectorAnimationData(AnimationData* animationData);
		void DrawInspectorKeyFrameProperties(KeyFrameProperties* properties);
		void DrawInspectorKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames);
		void DrawInspectorAetLayer(AetSet* aetSet, AetLayer* aetLayer);
		void DrawInspectorAetLyo(AetSet* aetSet, AetLyo* aetLyo);
		void DrawInspectorAetRegion(AetSet* aetSet, AetRegion* aetRegion);
	};
}