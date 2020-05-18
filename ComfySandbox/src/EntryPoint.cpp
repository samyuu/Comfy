#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"

#include "Window/ApplicationHost.h"
#include "Window/RenderWindow.h"
#include "ImGui/Gui.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Render/Render.h"
#include "Graphics/Auth2D/SprSet.h"

#include "IO/Path.h"
#include "IO/File.h"

#include "ImGui/Extensions/PropertyEditor.h"
#include "ImGui/Widgets/FileViewer.h"

#include <functional>

//#define COMFY_REGISTER_TEST_TASK(derivedClass) static inline struct StaticInit { StaticInit() { TestTaskInitializer::Register<derivedClass>(__FUNCTION__ "::" #derivedClass); } } InitInstance;
#define COMFY_REGISTER_TEST_TASK(derivedClass) static inline struct StaticInit { StaticInit() { TestTaskInitializer::Register<derivedClass>("Comfy::Sandbox::Tests::" #derivedClass); } } InitInstance;
// #define COMFY_REGISTER_TEST_TASK(derivedClass) COMFY_CALL_ON_STARTUP([] { TestTaskInitializer::Register<derivedClass>("Comfy::Sandbox::Tests::" #derivedClass); });

namespace Comfy::Sandbox::Tests
{
	class ITestTask
	{
	public:
		virtual ~ITestTask() = default;
		virtual void Update() = 0;
	};

	class TestTaskInitializer
	{
	public:
		const char* DerivedName;
		std::function<std::unique_ptr<ITestTask>()> Function;

		template <typename Func>
		static void IterateRegistered(Func func) { std::for_each(registered.begin(), registered.end(), func); }

		template <typename TestTask>
		static void Register(const char* derivedName) { registered.push_back({ derivedName, [] { return std::make_unique<TestTask>(); } }); }

	private:
		static inline std::vector<TestTaskInitializer> registered;
	};

	class AudioTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(AudioTest);

		AudioTest()
		{
			Audio::Engine::GetInstance().OpenStream();
			Audio::Engine::GetInstance().StartStream();
			// Audio::Engine::GetInstance().SetAudioAPI(Audio::Engine::AudioAPI::ASIO);
		}

		void Update() override
		{
			for (const auto keyCode : std::array {
				Input::KeyCode_W, Input::KeyCode_A, Input::KeyCode_S, Input::KeyCode_D,
				Input::KeyCode_I, Input::KeyCode_J, Input::KeyCode_K, Input::KeyCode_L, })
				if (Gui::IsKeyPressed(keyCode, false))
					Audio::Engine::GetInstance().PlaySound(buttonSound, "TestSound");
		}

	private:
		Audio::SourceHandle buttonSound = Audio::Engine::GetInstance().LoadAudioSource("dev_ram/sound/button/01_button1.wav");
	};

	class Renderer2DTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(Renderer2DTest);

		Renderer2DTest()
		{
			renderWindow.SetKeepAspectRatio(true);
			testCommand.SourceRegion = vec4(0, 0, 2048, 2048);

			renderWindow.OnRenderDebugFunc = [&]
			{
				camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				renderer.Begin(camera);
				{
					testCommand.Texture = (sprSet == nullptr || !InBounds(textureIndex, sprSet->TexSet->Textures)) ? nullptr : sprSet->TexSet->Textures[textureIndex].get();
					renderer.Draw(testCommand);
				}
				renderer.End();
			};
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

				if (Gui::Begin("RenderCommand2D Test"))
				{
					auto columns = GuiPropertyRAII::PropertyValueColumns();
					GuiProperty::Input("Texture Index", textureIndex);
					GuiProperty::Input("Origin", testCommand.Origin);
					GuiProperty::Input("Position", testCommand.Position);
					GuiProperty::Input("Rotation", testCommand.Rotation);
					GuiProperty::Input("Scale", testCommand.Scale, 0.05f);
					GuiProperty::Input("SourceRegion", testCommand.SourceRegion);
					GuiProperty::ColorEdit("CornerColors[0]", testCommand.CornerColors[0]);
					GuiProperty::ColorEdit("CornerColors[1]", testCommand.CornerColors[1]);
					GuiProperty::ColorEdit("CornerColors[2]", testCommand.CornerColors[2]);
					GuiProperty::ColorEdit("CornerColors[3]", testCommand.CornerColors[3]);
					GuiProperty::Checkbox("Draw Text Border", testCommand.DrawTextBorder);
				}
				Gui::End();

				if (Gui::Begin("Test SprSet Loader"))
				{
					if (sprFileViewer.DrawGui() && IO::Path::GetExtension(sprFileViewer.GetFileToOpen()) == ".bin")
						sprSet = IO::File::Load<Graphics::SprSet>(sprFileViewer.GetFileToOpen());
				}
				Gui::End();
			}
		}

	private:
		Render::Renderer2D renderer = {};

		Render::OrthographicCamera camera = {};

		Comfy::RenderWindow2D renderWindow = {};
		bool fullscreen = false;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset" };
		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_fnt/spr_fnt_36.bin");
		int textureIndex = 0;

		Render::RenderCommand2D testCommand;
	};

	class AetRendererTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(AetRendererTest);

		AetRendererTest()
		{
			renderWindow.SetKeepAspectRatio(true);

			renderWindow.OnRenderDebugFunc = [&]
			{
				camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				renderer.Begin(camera);
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

		Comfy::RenderWindow2D renderWindow = {};
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

	class Renderer3DTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(Renderer3DTest);

		Renderer3DTest()
		{
		}

		void Update() override
		{
		}

	private:
	};
}

namespace Comfy::Sandbox
{
	void Main()
	{
		ApplicationHost::ConstructionParam param = {};
		param.WindowTitle = "YEP COCK";
		param.IconHandle = nullptr;

		auto host = ApplicationHost(param);
		host.SetSwapInterval(1);

		std::unique_ptr<Tests::ITestTask> currentTestTask = std::make_unique<Tests::AetRendererTest>();
		host.EnterProgramLoop([&]
		{
			constexpr auto returnKey = Input::KeyCode_Escape;
			if (Gui::IsKeyPressed(returnKey, false))
				currentTestTask = nullptr;

			if (Gui::IsKeyPressed(Input::KeyCode_F1))
				host.SetSwapInterval(1);
			if (Gui::IsKeyPressed(Input::KeyCode_F2))
				host.SetSwapInterval(0);

			if (currentTestTask == nullptr)
			{
				const auto viewport = Gui::GetMainViewport();
				Gui::SetNextWindowPos(viewport->Pos);
				Gui::SetNextWindowSize(viewport->Size);
				Gui::SetNextWindowViewport(viewport->ID);

				const auto fullscreenWindowFlags = (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
				if (Gui::Begin("TestTaskSelection", nullptr, fullscreenWindowFlags))
				{
					Gui::TextUnformatted("Available ITestTasks:");
					Gui::SameLine();
					Gui::TextDisabled("(Return to TestTaskSelection by pressing '%s')", Input::GetKeyCodeName(returnKey));
					Gui::Separator();
					Tests::TestTaskInitializer::IterateRegistered([&](const auto& initializer)
					{
						if (Gui::Selectable(initializer.DerivedName))
							currentTestTask = initializer.Function();
					});
				}
				Gui::End();
			}
			else
			{
				currentTestTask->Update();
			}

			// Gui::DEBUG_NOSAVE_WINDOW("ShowStyleEditor", [&] { Gui::ShowStyleEditor(); }, ImGuiWindowFlags_None);
			// Gui::DEBUG_NOSAVE_WINDOW("ShowDemoWindow", [&] { Gui::ShowDemoWindow(); }, ImGuiWindowFlags_None);
		});
	}
}

int main(int argc, const char* argv[])
{
	Comfy::Sandbox::Main();
	return EXIT_SUCCESS;
}
