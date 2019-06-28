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

		std::unique_ptr<SprSet> sprSet;
		Renderer2D renderer;

		int currentBlendItem = (int)AetBlendMode::Alpha;
		int txpIndex = 0;
	};
}