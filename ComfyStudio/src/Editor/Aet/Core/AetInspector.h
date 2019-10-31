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
		char compNameBuffer[255];
		char layerNameBuffer[255];
		char markerNameBuffer[255];
		char spriteNameBuffer[255];
		char compDataNameBuffer[255];
		char surfaceDataNameBuffer[255];

		vec4 animatedPropertyColor, keyFramePropertyColor, staticPropertyColor;

	private:
		bool isPlayback = false;
		float currentFrame = 0.0f;

		AetRenderPreviewData* previewData = nullptr;

	private:
		void DrawInspectorAetSet(const RefPtr<Graphics::AetSet>& aetSet);
		void DrawInspectorAet(const RefPtr<Graphics::Aet>& aet);
		
		void DrawInspectorComposition(Graphics::Aet* aet, const RefPtr<Graphics::AetComposition>& comp);
		void DrawInspectorCompositionData(Graphics::Aet* aet, const RefPtr<Graphics::AetLayer>& layer, const RefPtr<Graphics::AetComposition>& comp);
		
		void DrawInspectorLayer(Graphics::Aet* aet, const RefPtr<Graphics::AetLayer>& layer);
		void DrawInspectorSurfaceData(Graphics::Aet* aet, const RefPtr<Graphics::AetLayer>& layer, const RefPtr<Graphics::AetSurface>& surface);
		
		void DrawInspectorAnimationData(const RefPtr<Graphics::AetAnimationData>& animationData, const RefPtr<Graphics::AetLayer>& layer);
		void DrawInspectorDebugAnimationData(const RefPtr<Graphics::AetAnimationData>& animationData, const RefPtr<Graphics::AetLayer>& layer);

		void DrawInspectorAnimationDataProperty(const RefPtr<Graphics::AetLayer>& layer, const char* label, frame_t frame, float& value, int propertyType);
		void DrawInspectorAnimationDataPropertyVec2(const RefPtr<Graphics::AetLayer>& layer, const char* label, frame_t frame, vec2& value, int propertyTypeX, int propertyTypeY);

		void DrawInspectorLayerMarkers(const RefPtr<Graphics::AetLayer>& layer, std::vector<RefPtr<Graphics::AetMarker>>* markers);
		void DrawInspectorLayerParent(Graphics::Aet* aet, const RefPtr<Graphics::AetLayer>& layer);
		
		void DrawInspectorSurface(Graphics::Aet* aet, const RefPtr<Graphics::AetSurface>& surface);

	private:
		void PushDisableItemFlagIfPlayback();
		void PopDisableItemFlagIfPlayback();
	};
}