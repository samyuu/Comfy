#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class AetRendererTest : public ITestTask
	{
	public:
		AetRendererTest()
		{
			// renderWindow.SetKeepAspectRatio(true);
			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				if (auto selectedScene = GetSelectedScene(); selectedScene != nullptr)
					camera.CenterAndZoomToFit(selectedScene->Resolution);

				renderer.Begin(camera, *renderWindow.RenderTarget);
				if (const auto selectedLayer = GetSelectedLayer(); selectedLayer != nullptr)
				{
					renderer.Aet().DrawLayerLooped(*selectedLayer, playback.CurrentFrame, playback.Transform);
				}
				renderer.End();

				if (playback.Playback)
					playback.CurrentFrame += Gui::GetIO().DeltaTime * playback.PlaybackFrameRate;
			};

			renderer.Aet().SetSprGetter([&](const Graphics::Aet::VideoSource& source)
			{
				return Render::SprSetNameStringSprGetter(source, sprSet.get());
			});
		}

		void Update() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_F11))
				fullscreen ^= true;

			if (fullscreen)
			{
				renderWindow.BeginEndGuiFullScreen("RenderWindow2D Test##FullSceen");
			}
			else
			{
				renderWindow.BeginEndGui("RenderWindow2D Test");

				if (Gui::Begin("Aet Control Test"))
				{
					auto columns = GuiPropertyRAII::PropertyValueColumns();
					GuiProperty::Input("Origin", playback.Transform.Origin);
					GuiProperty::Input("Position", playback.Transform.Position);
					GuiProperty::Input("Rotation", playback.Transform.Rotation);
					GuiProperty::Input("Scale", playback.Transform.Scale, 0.1f);
					GuiProperty::Input("Opacity", playback.Transform.Opacity, 0.01f, vec2(0.0f, 1.0f));

					GuiProperty::Input("Frame", playback.CurrentFrame);
					GuiProperty::Checkbox("Playback", playback.Playback);
					GuiProperty::Input("MSAA", renderWindow.RenderTarget->Param.MultiSampleCount);
				}
				Gui::End();

				if (Gui::Begin("Test SprSet Loader"))
				{
					if (sprFileViewer.DrawGui() && IO::Path::GetExtension(sprFileViewer.GetFileToOpen()) == ".bin")
					{
						sprSet = IO::File::Load<Graphics::SprSet>(sprFileViewer.GetFileToOpen());

						if (aetSet != nullptr)
							aetSet->ClearSpriteCache();
					}
				}
				Gui::End();

				if (Gui::Begin("Test AetSet Loader"))
				{
					if (aetFileViewer.DrawGui() && IO::Path::GetExtension(aetFileViewer.GetFileToOpen()) == ".bin")
						aetSet = IO::File::Load<Graphics::AetSet>(aetFileViewer.GetFileToOpen());
				}
				Gui::End();

				if (Gui::Begin("Test Composition Selection"))
				{
					if (const auto selectedScene = GetSelectedScene(); selectedScene != nullptr)
					{
						if (Gui::Selectable("Root", (selected.CompIndex < 0)))
							selected.CompIndex = -1;

						const auto selectedComp = GetSelectedComp();
						int compIndex = 0;

						for (const auto& comp : selectedScene->Compositions)
						{
							Gui::PushID(comp.get());
							if (Gui::Selectable(comp->GetName().data(), (comp.get() == selectedComp)))
								selected.CompIndex = compIndex;
							Gui::PopID();
							compIndex++;
						}
					}
					else
					{
						Gui::TextDisabled("<Invalid>");
					}
				}
				Gui::End();

				if (Gui::Begin("Test Layer Selection"))
				{
					if (const auto selectedComp = GetSelectedComp(); selectedComp != nullptr)
					{
						selected.LayerHoverIndex = -1;
						const auto selectedLayer = GetSelectedLayer();
						int layerIndex = 0;

						for (const auto& layer : selectedComp->GetLayers())
						{
							Gui::PushID(layer.get());

							if (Gui::Selectable(layer->GetName().data(), (layer.get() == selectedLayer)))
								selected.LayerIndex = layerIndex;

							if (Gui::IsItemHovered())
							{
								selected.LayerHoverIndex = layerIndex;
								if (auto previewLayer = GetPreviewLayer(); previewLayer != nullptr)
								{
									constexpr float targetSize = 360.0f;
									const auto sceneSize = vec2(GetSelectedScene()->Resolution);
									const auto size = vec2(targetSize, targetSize * (sceneSize.y / sceneSize.x));

									preview.Camera.ProjectionSize = size;
									preview.RenderTarget->Param.Resolution = size;
									preview.Camera.Zoom = (targetSize / sceneSize.x);

									renderer.Begin(preview.Camera, *preview.RenderTarget);
									renderer.Aet().DrawLayerLooped(*previewLayer, playback.CurrentFrame, playback.Transform);
									renderer.End();

									Gui::BeginTooltip();
									Gui::Image(preview.RenderTarget->GetTextureID(), size);
									Gui::EndTooltip();
								}
							}

							Gui::PopID();
							layerIndex++;
						}
					}
					else
					{
						Gui::TextDisabled("<Invalid>");
					}
				}
				Gui::End();
			}
		}

	private:
		Graphics::Aet::Scene* GetSelectedScene() const
		{
			return (aetSet != nullptr) ? aetSet->GetScenes().front().get() : nullptr;
		}

		Graphics::Aet::Composition* GetSelectedComp() const
		{
			auto selectedScene = GetSelectedScene();
			if (selectedScene == nullptr)
				return nullptr;

			if (selected.CompIndex < 0)
				return selectedScene->RootComposition.get();
			else
				return InBounds(selected.CompIndex, selectedScene->Compositions) ? selectedScene->Compositions[selected.CompIndex].get() : nullptr;
		}

		Graphics::Aet::Layer* GetSelectedLayer() const
		{
			auto selectedComp = GetSelectedComp();
			if (selectedComp == nullptr)
				return nullptr;

			return InBounds(selected.LayerIndex, selectedComp->GetLayers()) ? selectedComp->GetLayers()[selected.LayerIndex].get() : nullptr;
		}

		Graphics::Aet::Layer* GetPreviewLayer() const
		{
			auto selectedComp = GetSelectedComp();
			if (selectedComp == nullptr)
				return nullptr;

			return (InBounds(selected.LayerHoverIndex, selectedComp->GetLayers())) ? selectedComp->GetLayers()[selected.LayerHoverIndex].get() : nullptr;
		}

	private:
		Render::Renderer2D renderer = {};
		Render::Camera2D camera = {};

		CallbackRenderWindow2D renderWindow = {};
		bool fullscreen = false;

		struct PreviewData
		{
			Render::Camera2D Camera = {};
			std::unique_ptr<Render::RenderTarget2D> RenderTarget = Render::Renderer2D::CreateRenderTarget();
		} preview;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };
		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };

		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_ps4/spr_ps4_adv.bin");
		std::unique_ptr<Graphics::AetSet> aetSet = IO::File::Load<Graphics::AetSet>("dev_ram/aetset/aet_ps4/aet_ps4_adv.bin");

		struct PlaybackData
		{
			Graphics::Transform2D Transform = { vec2(0.0f) };

			bool Playback = true;
			frame_t CurrentFrame = 0.0f;
			frame_t PlaybackFrameRate = 60.0f;

		} playback;

		struct SelectedData
		{
			int CompIndex = -1;
			int LayerIndex = 0;

			int LayerHoverIndex = -1;
		} selected;
	};
}
