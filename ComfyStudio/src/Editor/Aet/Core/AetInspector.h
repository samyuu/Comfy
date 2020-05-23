#pragma once
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/MutatingEditorComponent.h"
#include "Editor/Aet/RenderWindow/AetRenderPreviewData.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class AetInspector : public MutatingEditorComponent
	{
	public:
		AetInspector(AetCommandManager& commandManager, Render::Renderer2D& renderer, AetRenderPreviewData& previewData);
		~AetInspector() = default;

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

		Render::Renderer2D& renderer;
		AetRenderPreviewData& previewData;

	private:
		void DrawInspectorAetSet(const std::shared_ptr<Graphics::Aet::AetSet>& aetSet);
		void DrawInspectorScene(const std::shared_ptr<Graphics::Aet::Scene>& scene);
		
		void DrawInspectorComposition(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Composition>& comp);
		void DrawInspectorCompositionData(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Layer>& layer, const std::shared_ptr<Graphics::Aet::Composition>& comp);
		
		void DrawInspectorLayer(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Layer>& layer);
		void DrawInspectorVideoData(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Layer>& layer, const std::shared_ptr<Graphics::Aet::Video>& video);
		
		void DrawInspectorAnimationData(const std::shared_ptr<Graphics::Aet::LayerVideo>& animationData, const std::shared_ptr<Graphics::Aet::Layer>& layer);
		void DrawInspectorDebugAnimationData(const std::shared_ptr<Graphics::Aet::LayerVideo>& animationData, const std::shared_ptr<Graphics::Aet::Layer>& layer);

		void DrawInspectorAnimationDataProperty(const std::shared_ptr<Graphics::Aet::Layer>& layer, const char* label, frame_t frame, float& value, Graphics::Transform2DField field);
		void DrawInspectorAnimationDataPropertyVec2(const std::shared_ptr<Graphics::Aet::Layer>& layer, const char* label, frame_t frame, vec2& value, Graphics::Transform2DField fieldX, Graphics::Transform2DField fieldY);

		void DrawInspectorLayerMarkers(const std::shared_ptr<Graphics::Aet::Layer>& layer, std::vector<std::shared_ptr<Graphics::Aet::Marker>>* markers);
		void DrawInspectorLayerParent(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Layer>& layer);
		
		void DrawInspectorVideo(Graphics::Aet::Scene* scene, const std::shared_ptr<Graphics::Aet::Video>& video);

	private:
		void PushDisableItemFlagIfPlayback();
		void PopDisableItemFlagIfPlayback();
	};
}
