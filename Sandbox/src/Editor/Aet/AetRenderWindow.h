#pragma once
#include "../RenderWindowBase.h"
#include "../../FileSystem/File/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AetRenderWindow : public RenderWindowBase
	{
	public:
		AetRenderWindow();
		~AetRenderWindow();

		void SetAetObj(AetObj* value);

	protected:
		void OnUpdateInput() override;
		void OnUpdate()  override;
		void OnRender()  override;
		void OnResize(int width, int height) override;
	
	private:
		AetObj* aetObj;
	};
}