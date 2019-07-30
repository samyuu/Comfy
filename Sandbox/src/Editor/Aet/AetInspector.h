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
		bool DrawGui(Aet* aet, const AetItemTypePtr& selected);

	private:

		char aetNameBuffer[255];
		char aetObjNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];
		char layerDataNameBuffer[255];
		char regionDataNameBuffer[255];

		void DrawInspectorAetSet(AetSet* aetSet);
		void DrawInspectorAet(Aet* aet);
		
		void DrawInspectorAetLayer(Aet* aet, AetLayer* aetLayer);
		void DrawInspectorLayerData(Aet* aet, AetObj* aetObj, AetLayer* aetLayer);
		
		void DrawInspectorAetObj(Aet* aet, AetObj* aetObj);
		void DrawInspectorRegionData(Aet* aet, AetObj* aetObj, AetRegion* spriteEntry);
		void DrawInspectorAnimationData(AnimationData* animationData);
		void DrawInspectorKeyFrameProperties(KeyFrameProperties* properties);
		void DrawInspectorKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames);
		void DrawInspectorAetObjMarkers(std::vector<Marker>* markers);
		void DrawInspectorAetObjParent(AetObj* aetObj);
		
		void DrawInspectorAetRegion(Aet* aet, AetRegion* aetRegion);
	};
}