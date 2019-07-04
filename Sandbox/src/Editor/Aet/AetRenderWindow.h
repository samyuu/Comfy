#pragma once
#include "Editor/RenderWindowBase.h"
#include "FileSystem/Format/AetSet.h"
#include "ImGui/Widgets/FileViewer.h"
#include "FileSystem/Format/SprSet.h"
#include "Graphics/Auth2D/Renderer2D.h"

namespace Editor
{
	using namespace FileSystem;
	using namespace Auth2D;

	class AetRenderWindow : public RenderWindowBase
	{
	public:
		AetRenderWindow();
		~AetRenderWindow();

		void SetAetLyo(AetLyo* value);
		void SetAetObj(AetObj* value);

	protected:
		void OnDrawGui() override;
		void OnUpdateInput() override;
		void OnUpdate()  override;
		void OnRender()  override;
		void OnResize(int width, int height) override;

	protected:
		void OnInitialize() override;

	private:
		ImGui::FileViewer fileViewer = { "dev_ram/sprset/" };

		AetLyo* aetLyo = nullptr;
		AetObj* aetObj = nullptr;

		vec2 newRendererSize;
		std::unique_ptr<SprSet> sprSet;
		Renderer2D renderer;

		struct
		{
			vec4 aetSourceRegion = { 0.0f, 0.0f, 0.0f, 0.0f };
			vec4 aetColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			vec2 aetPosition = { 100, 100 };
			vec2 aetOrigin = { };
			vec2 aetScale = { 1.0f, 1.0f };
			float aetRotation = { };
		};

		bool useTextShadow = false;
		int currentBlendItem = (int)AetBlendMode::Alpha;
		int txpIndex = 0, spriteIndex = -1;
	};
}