#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class AetRendererTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(AetRendererTest);

		AetRendererTest()
		{
			renderWindow.SetKeepAspectRatio(true);

			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				renderer.Begin(camera, *renderWindow.RenderTarget);
				if (const auto selectedLayer = GetSelectedLayer(); selectedLayer != nullptr)
					renderer.Aet().DrawLayerLooped(*selectedLayer, playback.CurrentFrame, playback.Position, playback.Opacity);
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
					GuiProperty::Input("Position", playback.Position);
					GuiProperty::Input("Opacity", playback.Opacity, 0.01f, vec2(0.0f, 1.0f));

					GuiProperty::Input("Frame", playback.CurrentFrame);
					GuiProperty::Checkbox("Playback", playback.Playback);
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
						aetSet = IO::File::Load<Graphics::Aet::AetSet>(aetFileViewer.GetFileToOpen());
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
								selected.LayerHoverIndex = layerIndex;

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

			if (InBounds(selected.LayerHoverIndex, selectedComp->GetLayers()))
				return selectedComp->GetLayers()[selected.LayerHoverIndex].get();

			return InBounds(selected.LayerIndex, selectedComp->GetLayers()) ? selectedComp->GetLayers()[selected.LayerIndex].get() : nullptr;
		}

	private:
		Render::Renderer2D renderer = {};
		Render::OrthographicCamera camera = {};

		Comfy::CallbackRenderWindow2D renderWindow = {};
		bool fullscreen = false;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };
		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };

		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_ps4/spr_ps4_adv.bin");
		std::unique_ptr<Graphics::Aet::AetSet> aetSet = IO::File::Load<Graphics::Aet::AetSet>("dev_ram/aetset/aet_ps4/aet_ps4_adv.bin");

		struct PlaybackData
		{
			vec2 Position = vec2(0.0f, 0.0f);
			float Opacity = 1.0f;

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
