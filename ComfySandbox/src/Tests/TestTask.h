#pragma once
#include "Types.h"

#include "Window/RenderWindow.h"
#include "ImGui/Gui.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Render/Render.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth2D/Font/FontMap.h"
#include "Time/Stopwatch.h"

#include "IO/Path.h"
#include "IO/File.h"

#include "ImGui/Extensions/PropertyEditor.h"
#include "ImGui/Widgets/FileViewer.h"

#include <functional>

namespace Comfy::Sandbox::Tests
{
	// DEBUG:
	class CallbackRenderWindow2D : public RenderWindow
	{
	public:
		CallbackRenderWindow2D() = default;
		~CallbackRenderWindow2D() = default;

	public:
		ImTextureID GetTextureID() const override { return (RenderTarget != nullptr) ? RenderTarget->GetTextureID() : nullptr; }

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override { return ImGuiWindowFlags_None; }
		void PreRenderTextureGui() override {}
		void PostRenderTextureGui() override {}
		void OnResize(ivec2 newSize) override {}
		void OnRender() override { OnRenderCallback(); }

	public:
		std::function<void()> OnRenderCallback;
		std::unique_ptr<Render::RenderTarget2D> RenderTarget = Render::Renderer2D::CreateRenderTarget();
	};

	class ITestTask
	{
	public:
		virtual ~ITestTask() = default;
		virtual void Update() = 0;
	};

	struct TestTaskInitializer
	{
		template <typename DerivedTask>
		static TestTaskInitializer Create(std::string_view name)
		{
			TestTaskInitializer result;
			result.Name = name;
			result.Function = [] { return std::make_unique<DerivedTask>(); };
			return result;
		}

		std::string_view Name;
		std::function<std::unique_ptr<ITestTask>()> Function;
	};
}
