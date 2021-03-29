#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Input/Input.h"

namespace Comfy::Studio::DataTest
{
	class InputTestWindow : public BaseWindow
	{
	public:
		InputTestWindow(ComfyStudioApplication&);
		~InputTestWindow() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	private:
	};
}
