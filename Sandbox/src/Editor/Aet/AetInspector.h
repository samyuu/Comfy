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
		char aetNameBuffer[255];
		char aetObjNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];

		void DrawInspectorAetSet(AetSet* aetSet);
		void DrawInspectorAet(AetSet* aetSet, Aet* aet);
		
		void DrawInspectorAetLayer(AetSet* aetSet, AetLayer* aetLayer);
		void DrawInspectorLayerData(AetLayer* aetLayer);
		
		void DrawInspectorAetObj(AetSet* aetSet, AetObj* aetObj);
		void DrawInspectorRegionData(AetRegion* spriteEntry);
		void DrawInspectorAnimationData(AnimationData* animationData);
		void DrawInspectorKeyFrameProperties(KeyFrameProperties* properties);
		void DrawInspectorKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames);
		void DrawInspectorAetObjMarkers(std::vector<Marker>* markers);
		void DrawInspectorAetObjParent(AetObj* aetObj);
		
		void DrawInspectorAetRegion(AetSet* aetSet, AetRegion* aetRegion);
	};
}