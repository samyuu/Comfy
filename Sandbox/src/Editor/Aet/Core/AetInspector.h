#pragma once
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/IMutableAetEditorComponent.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AetInspector : public IMutableAetEditorComponent
	{
	public:
		AetInspector(AetCommandManager* commandManager);
		~AetInspector();

		void Initialize();
		bool DrawGui(Aet* aet, const AetItemTypePtr& selected);

	private:
		AetItemTypePtr lastSelectedItem;
		int newParentObjLayerIndex = -1;

		char aetNameBuffer[255];
		char aetObjNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];
		char layerDataNameBuffer[255];
		char regionDataNameBuffer[255];
		char parentObjDataNameBuffer[255];

		void DrawInspectorAetSet(const RefPtr<AetSet>& aetSet);
		void DrawInspectorAet(const RefPtr<Aet>& aet);
		
		void DrawInspectorAetLayer(Aet* aet, const RefPtr<AetLayer>& aetLayer);
		void DrawInspectorLayerData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetLayer>& aetLayer);
		
		void DrawInspectorAetObj(Aet* aet, const RefPtr<AetObj>& aetObj);
		void DrawInspectorRegionData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetRegion>& spriteEntry);
		void DrawInspectorAnimationData(const RefPtr<AnimationData>& animationData, AetObjType objType);
		void DrawInspectorKeyFrameProperties(KeyFrameProperties* properties);
		void DrawInspectorKeyFrames(const char* name, Vector<AetKeyFrame>* keyFrames);
		void DrawInspectorAetObjMarkers(const RefPtr<AetObj>& aetObj, Vector<RefPtr<AetMarker>>* markers);
		void DrawInspectorAetObjParent(Aet* aet, const RefPtr<AetObj>& aetObj);
		
		void DrawInspectorAetRegion(Aet* aet, const RefPtr<AetRegion>& aetRegion);
	};
}