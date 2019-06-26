#pragma once
#include "Editor/RenderWindowBase.h"
#include "FileSystem/Format/AetSet.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Editor
{
	using namespace FileSystem;

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
	
	private:
		ImGui::FileViewer fileViewer { "dev_ram/sprset/" };

		AetLyo* aetLyo = nullptr;
		AetObj* aetObj = nullptr;
	};
}