#pragma once
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/RenderWindow/AetRenderPreviewData.h"
#include "Graphics/Auth2D/AetSet.h"

namespace Editor
{
	class AetInspector : public IMutatingEditorComponent
	{
	public:
		AetInspector(AetCommandManager* commandManager, AetRenderPreviewData* previewData);
		~AetInspector();

		void Initialize();
		bool DrawGui(const AetItemTypePtr& selected);

		void SetIsPlayback(bool value);
		float SetCurrentFrame(float value);

	private:
		AetItemTypePtr lastSelectedItem;

		char aetNameBuffer[255];
		char layerNameBuffer[255];
		char aetObjNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];
		char layerDataNameBuffer[255];
		char regionDataNameBuffer[255];

		vec4 animatedPropertyColor, keyFramePropertyColor, staticPropertyColor;

	private:
		bool isPlayback = false;
		float currentFrame = 0.0f;

		AetRenderPreviewData* previewData = nullptr;

	private:
		void DrawInspectorAetSet(const RefPtr<Graphics::AetSet>& aetSet);
		void DrawInspectorAet(const RefPtr<Graphics::Aet>& aet);
		
		void DrawInspectorAetLayer(Graphics::Aet* aet, const RefPtr<Graphics::AetLayer>& aetLayer);
		void DrawInspectorLayerData(Graphics::Aet* aet, const RefPtr<Graphics::AetObj>& aetObj, const RefPtr<Graphics::AetLayer>& aetLayer);
		
		void DrawInspectorAetObj(Graphics::Aet* aet, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawInspectorRegionData(Graphics::Aet* aet, const RefPtr<Graphics::AetObj>& aetObj, const RefPtr<Graphics::AetRegion>& spriteEntry);
		
		void DrawInspectorAnimationData(const RefPtr<Graphics::AnimationData>& animationData, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawInspectorDebugAnimationData(const RefPtr<Graphics::AnimationData>& animationData, const RefPtr<Graphics::AetObj>& aetObj);

		void DrawInspectorAnimationDataProperty(const RefPtr<Graphics::AetObj>& aetObj, const char* label, frame_t frame, float& value, int propertyType);
		void DrawInspectorAnimationDataPropertyVec2(const RefPtr<Graphics::AetObj>& aetObj, const char* label, frame_t frame, vec2& value, int propertyTypeX, int propertyTypeY);

		void DrawInspectorAetObjMarkers(const RefPtr<Graphics::AetObj>& aetObj, std::vector<RefPtr<Graphics::AetMarker>>* markers);
		void DrawInspectorAetObjParent(Graphics::Aet* aet, const RefPtr<Graphics::AetObj>& aetObj);
		
		void DrawInspectorAetRegion(Graphics::Aet* aet, const RefPtr<Graphics::AetRegion>& aetRegion);

	private:
		void PushDisableItemFlagIfPlayback();
		void PopDisableItemFlagIfPlayback();
	};
}