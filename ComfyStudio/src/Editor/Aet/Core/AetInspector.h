#pragma once
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/RenderWindow/AetRenderPreviewData.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "Graphics/Auth2D/Aet/AetRenderer.h"

namespace Comfy::Editor
{
	class AetInspector : public IMutatingEditorComponent
	{
	public:
		AetInspector(AetCommandManager* commandManager, Graphics::Aet::SpriteGetterFunction* spriteGetter, AetRenderPreviewData* previewData);
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
		char videoDataNameBuffer[255];

		vec4 animatedPropertyColor, keyFramePropertyColor, staticPropertyColor;

	private:
		bool isPlayback = false;
		float currentFrame = 0.0f;

		Graphics::Aet::SpriteGetterFunction* spriteGetter;
		AetRenderPreviewData* previewData = nullptr;

	private:
		void DrawInspectorAetSet(const RefPtr<Graphics::Aet::AetSet>& aetSet);
		void DrawInspectorAet(const RefPtr<Graphics::Aet::Scene>& scene);
		
		void DrawInspectorComposition(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Composition>& comp);
		void DrawInspectorCompositionData(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Layer>& layer, const RefPtr<Graphics::Aet::Composition>& comp);
		
		void DrawInspectorLayer(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Layer>& layer);
		void DrawInspectorVideoData(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Layer>& layer, const RefPtr<Graphics::Aet::Video>& video);
		
		void DrawInspectorAnimationData(const RefPtr<Graphics::Aet::LayerVideo>& animationData, const RefPtr<Graphics::Aet::Layer>& layer);
		void DrawInspectorDebugAnimationData(const RefPtr<Graphics::Aet::LayerVideo>& animationData, const RefPtr<Graphics::Aet::Layer>& layer);

		void DrawInspectorAnimationDataProperty(const RefPtr<Graphics::Aet::Layer>& layer, const char* label, frame_t frame, float& value, Graphics::Transform2DField field);
		void DrawInspectorAnimationDataPropertyVec2(const RefPtr<Graphics::Aet::Layer>& layer, const char* label, frame_t frame, vec2& value, Graphics::Transform2DField fieldX, Graphics::Transform2DField fieldY);

		void DrawInspectorLayerMarkers(const RefPtr<Graphics::Aet::Layer>& layer, std::vector<RefPtr<Graphics::Aet::Marker>>* markers);
		void DrawInspectorLayerParent(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Layer>& layer);
		
		void DrawInspectorVideo(Graphics::Aet::Scene* scene, const RefPtr<Graphics::Aet::Video>& video);

	private:
		void PushDisableItemFlagIfPlayback();
		void PopDisableItemFlagIfPlayback();
	};
}
