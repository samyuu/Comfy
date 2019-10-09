#pragma once
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AetInspector : public IMutatingEditorComponent
	{
	public:
		AetInspector(AetCommandManager* commandManager);
		~AetInspector();

		void Initialize();
		bool DrawGui(const AetItemTypePtr& selected);

		void SetIsPlayback(bool value);
		float SetCurrentFrame(float value);

	private:
		AetItemTypePtr lastSelectedItem;

		char aetNameBuffer[255];
		char aetObjNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];
		char layerDataNameBuffer[255];
		char regionDataNameBuffer[255];

		vec4 animatedPropertyColor, keyFramePropertyColor, staticPropertyColor;

	private:
		bool isPlayback = false;
		float currentFrame = 0.0f;

	private:
		void DrawInspectorAetSet(const RefPtr<AetSet>& aetSet);
		void DrawInspectorAet(const RefPtr<Aet>& aet);
		
		void DrawInspectorAetLayer(Aet* aet, const RefPtr<AetLayer>& aetLayer);
		void DrawInspectorLayerData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetLayer>& aetLayer);
		
		void DrawInspectorAetObj(Aet* aet, const RefPtr<AetObj>& aetObj);
		void DrawInspectorRegionData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetRegion>& spriteEntry);
		
		void DrawInspectorAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj);
		void DrawInspectorDebugAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj);

		void DrawInspectorAnimationDataProperty(const RefPtr<AetObj>& aetObj, const char* label, frame_t frame, float& value, int propertyType);
		void DrawInspectorAnimationDataPropertyVec2(const RefPtr<AetObj>& aetObj, const char* label, frame_t frame, vec2& value, int propertyTypeX, int propertyTypeY);

		void DrawInspectorAetObjMarkers(const RefPtr<AetObj>& aetObj, Vector<RefPtr<AetMarker>>* markers);
		void DrawInspectorAetObjParent(Aet* aet, const RefPtr<AetObj>& aetObj);
		
		void DrawInspectorAetRegion(Aet* aet, const RefPtr<AetRegion>& aetRegion);

	private:
		void PushDisableItemFlagIfPlayback();
		void PopDisableItemFlagIfPlayback();
	};
}