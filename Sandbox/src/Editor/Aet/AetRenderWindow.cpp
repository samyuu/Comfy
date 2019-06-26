#include "AetRenderWindow.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Buffer.h"
#include "Graphics/ComfyVertex.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture.h"
#include "FileSystem/Format/SprSet.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	using namespace FileSystem;

	// -----------------------------------------
	// --- TEMP:
	// -----------------------------------------
	static float textureScale = 1.0f;
	static SpriteShader shader;
	static VertexBuffer vertexBuffer;
	static VertexArray vertexArray;
	static std::unique_ptr<SprSet> spriteSet;

	void InitTex(const char* sprFilePath)
	{
		std::vector<uint8_t> fileBuffer;
		ReadAllBytes(sprFilePath, &fileBuffer);

		spriteSet = std::make_unique<SprSet>();
		spriteSet->Parse(fileBuffer.data());

		for (int i = 0; i < spriteSet->TxpSet->Textures.size(); i++)
		{
			spriteSet->TxpSet->Textures[i]->Texture2D = std::make_shared<Texture2D>();
			spriteSet->TxpSet->Textures[i]->Texture2D->Upload(spriteSet->TxpSet->Textures[i].get());
		}
	}
	void TempInit()
	{
		mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);

		shader.Initialize();
		shader.Bind();
		shader.SetUniform(shader.ProjectionLocation, projection);

		constexpr float margin = 30;
		constexpr vec4 color = vec4(1, 1, 1, 1);

		SpriteVertex vertices[] =
		{
			{ vec2(1280 - margin,   0 + margin), vec2(1, 0), color }, // tr
			{ vec2(1280 - margin, 720 - margin), vec2(1, 1), color }, // br
			{ vec2(0 + margin, 720 - margin), vec2(0, 1), color }, // bl

			{ vec2(0 + margin, 720 - margin), vec2(0, 1), color }, // bl
			{ vec2(0 + margin,   0 + margin), vec2(0, 0), color }, // tl
			{ vec2(1280 - margin,   0 + margin), vec2(1, 0), color }, // tr
		};

		vertexBuffer.InitializeID();
		vertexBuffer.Bind();
		vertexBuffer.Upload(vertices, sizeof(vertices), BufferUsage::StaticDraw);

		BufferLayout layout =
		{
			{ ShaderDataType::Vec2, "in_position" },
			{ ShaderDataType::Vec2, "in_texture_coords" },
			{ ShaderDataType::Vec4, "in_color" }
		};

		vertexArray.InitializeID();
		vertexArray.Bind();
		vertexArray.SetLayout(layout);

		//InitTex("dev_ram/sprset/spr_gam_cmn.bin");
	}
	void CheckTempInit()
	{
		static bool init = true;
		if (!init)
			return;

		init = false;
		TempInit();
	}
	// -----------------------------------------

	AetRenderWindow::AetRenderWindow()
	{
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetAetLyo(AetLyo* value)
	{
		aetLyo = value;
	}

	void AetRenderWindow::SetAetObj(AetObj* value)
	{
		aetObj = value;
	}

	void AetRenderWindow::OnDrawGui()
	{
		ImGui::Begin("SprSet Loader", nullptr, ImGuiWindowFlags_NoBackground);
		{
			ImGui::BeginChild("SprSetLoaderChild##AetRenderWindow");
			if (fileViewer.DrawGui())
			{
				std::string sprPath = fileViewer.GetFileToOpen();
				if (StartsWithInsensitive(GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
					InitTex(sprPath.c_str());
			}
			ImGui::EndChild();
		}
		ImGui::End();

		ImGui::Begin("Txp Preview", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
		{
			ImGui::SliderFloat("Texture Scale", &textureScale, 0.1f, 2.0f);
			if (spriteSet)
			{
				for (int i = 0; i < spriteSet->TxpSet->Textures.size(); i++)
				{
					auto tex = spriteSet->TxpSet->Textures[i].get();

					ImGui::PushID(&tex);
					ImGui::BeginChild("AetTexturePreviewChildInner", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
					if (ImGui::WideTreeNode(spriteSet->TxpSet->Textures[i]->Name.c_str()))
					{
						ImGui::Image(tex->Texture2D->GetVoidTexture(), ImVec2(tex->Texture2D->GetWidth() * textureScale, tex->Texture2D->GetHeight() * textureScale), ImGui::UV0_GL, ImGui::UV1_GL);
						ImGui::TreePop();
					}
					ImGui::EndChild();
					ImGui::PopID();
				}
			}
		}
		ImGui::End();
	}

	void AetRenderWindow::OnUpdateInput()
	{
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		CheckTempInit();

		renderTarget.Bind();
		{
			glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			ImVec4 backgroundColor = (aetLyo == nullptr) ? ImVec4(.4f, .5f, .1f, 1.0f) : ImGui::ColorConvertU32ToFloat4(aetLyo->BackgroundColor);
			glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
			glClear(GL_COLOR_BUFFER_BIT);

			vertexArray.Bind();
			shader.Bind();

			if (spriteSet && spriteSet->TxpSet->Textures.size() > 0)
			{
				auto* texture = spriteSet->TxpSet->Textures[0].get();
				auto format = spriteSet->TxpSet->Textures[0]->MipMaps[0]->Format;

				shader.SetUniform(shader.TextureFormatLocation, static_cast<int>(format));

				texture->Texture2D->Bind(0);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
		renderTarget.UnBind();
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		//if (shader.GetIsInitialized())
		//{
		//	auto size = GetRenderRegion().GetSize();
		//	mat4 projection = glm::ortho(0.0f, size.x, 0.0f, size.y, -1.0f, 1.0f);

		//	shader.Use();
		//	shader.SetUniform(shader.ProjectionLocation, projection);
		//}
	}
}
