#pragma once
#include "Core/BaseWindow.h"
#include "Graphics/OpenGL/GL_ShaderProgram.h"

namespace DataTest
{
	class ShaderTestWindow : public BaseWindow
	{
	public:
		ShaderTestWindow(Application*);
		~ShaderTestWindow();

		virtual void DrawGui() override;
		virtual const char* GetGuiName() const override;
		virtual ImGuiWindowFlags GetWindowFlags() const override;

	private:
		static constexpr std::array<const char*, static_cast<size_t>(Graphics::UniformType::Count)> typeNames =
		{
			"Int", 
			"Float", 
			"Vec2", 
			"Vec3", 
			"Vec4", 
			"Mat4",
		};

		const float listWidth = 0.2f;

		int selectedIndex = 0;
	};
}