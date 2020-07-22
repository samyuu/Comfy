#include "TestTask.h"
#include "Resource/ResourceIDMap.h"

namespace Comfy::Sandbox::Tests
{
	class Renderer3DTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(Renderer3DTest);

		Renderer3DTest()
		{
			sceneParam.IBL = IO::File::Load<Graphics::IBLParameters>("dev_rom/ibl/tst007.ibl");

			if (objSet != nullptr && texSet != nullptr)
			{
				texSet->SetTextureIDs(*objSet);

				for (auto& tex : texSet->Textures)
					texIDMap.Add(tex->ID, tex);
			}
		}

		void Update() override
		{
			if (Gui::Begin("Renderer3D Test"))
			{
				const vec2 size = Gui::GetContentRegionAvail();
				camera.AspectRatio = (size.x / size.y);
				renderTarget->Param.RenderResolution = size;

				renderer.Begin(camera, *renderTarget, sceneParam);
				{
					if (objSet != nullptr)
					{
						for (auto& obj : objSet->Objects)
							renderer.Draw(Render::RenderCommand3D(obj));
					}
				}
				renderer.End();

				Gui::Image(renderTarget->GetTextureID(), size);
			}
			Gui::End();
		}

	private:
		std::unique_ptr<Graphics::ObjSet> objSet = IO::File::Load<Graphics::ObjSet>("dev_rom/objset/copy/stgtst007/stgtst007_obj.bin");
		std::unique_ptr<Graphics::TexSet> texSet = IO::File::Load<Graphics::TexSet>("dev_rom/objset/copy/stgtst007/stgtst007_tex.bin");

		ResourceIDMap<TexID, Graphics::Tex> texIDMap;

		Render::Camera3D camera;
		Render::Renderer3D renderer = Render::TexGetter([&](const Cached_TexID* texID)
		{
			return texIDMap.Find(texID);
		});

		std::unique_ptr<Render::RenderTarget3D> renderTarget = Render::Renderer3D::CreateRenderTarget();
		Render::SceneParam3D sceneParam;
	};
}
