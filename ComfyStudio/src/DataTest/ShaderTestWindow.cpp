#include "ShaderTestWindow.h"
#include "ImGui/Gui.h"

namespace DataTest
{
	ShaderTestWindow::ShaderTestWindow(Application* parent) : BaseWindow(parent)
	{
		CloseWindow();
	}

	ShaderTestWindow::~ShaderTestWindow()
	{
	}

	void ShaderTestWindow::DrawGui()
	{
		using namespace ImGui;

		constexpr ImGuiWindowFlags scrollBarWindowFlags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

		Text("Shader Test:");

		BeginChild("##ShaderTestWindowListChild", vec2(GetWindowWidth() * listWidth, 0.0f), true, scrollBarWindowFlags);
		{
			int i = 0;
			for (auto& shader : Graphics::ShaderProgram::GetAllShaderPrograms())
			{
				PushID(shader);
				if (Gui::Selectable(shader->GetShaderName(), i == selectedIndex))
					selectedIndex = i;
				i++;
				PopID();
			}
		}
		EndChild();
		SameLine();
		BeginChild("##ShaderTestWindowInfoChild", vec2(0.0f, 0.0f), true);
		{
			if (selectedIndex >= 0 && selectedIndex < Graphics::ShaderProgram::GetAllShaderPrograms().size())
			{
				Graphics::ShaderProgram* shader = Graphics::ShaderProgram::GetAllShaderPrograms()[selectedIndex];

				BulletText("%s	[ OpenGL ID: %d ]", shader->GetShaderName(), shader->GetProgramID());
				BeginChild("##ShaderTestWindowSourceChild", vec2(0.0f, 82.0f), true);
				{
					Columns(2);

					Text("Shader Type");
					NextColumn();
					Text("Path");
					NextColumn();
					Separator();

					PushID(shader->GetVertexShaderPath());
					Selectable("##ShaderSourceSelectable::ShaderTestWindow", false, ImGuiSelectableFlags_SpanAllColumns);
					PopID();
					SameLine();

					Text("Vertex Shader");
					NextColumn();
					Text("%s", shader->GetVertexShaderPath());
					NextColumn();

					PushID(shader->GetVertexShaderPath());
					Selectable("##ShaderSourceSelectable::ShaderTestWindow", false, ImGuiSelectableFlags_SpanAllColumns);
					PopID();
					SameLine();

					Text("Fragment Shader");
					NextColumn();
					Text("%s", shader->GetFragmentShaderPath());
					NextColumn();

					Columns(1);

					if (Button("Reload", vec2(GetWindowWidth(), 0.0f)))
						shader->Recompile();
				}
				EndChild();

				BulletText("Uniforms");
				BeginChild("##ShaderTestWindowUniformChild", vec2(0.0f, 0.0f), true);
				{
					Columns(3);
					Text("Data Type");
					NextColumn();
					Text("Name");
					NextColumn();
					Text("Location");
					NextColumn();
					Separator();

					for (Graphics::Uniform* uniform = shader->GetFirstUniform(); uniform <= shader->GetLastUniform(); uniform++)
					{
						PushID(uniform);
						Selectable("##UniformSelectable::ShaderTestWindow", false, ImGuiSelectableFlags_SpanAllColumns);
						PopID();
						SameLine();

						const char* typeName = uniform->GetType() <= Graphics::UniformType::Count ? typeNames.at(static_cast<int32_t>(uniform->GetType())) : "Unknown";
						Text("%s", typeName);
						NextColumn();
						Text("%s", uniform->GetName());
						NextColumn();
						Text("%d", uniform->GetLocation());
						NextColumn();
					}

					Columns(1);
				}
				EndChild();
			}
		}
		EndChild();
	}

	const char* ShaderTestWindow::GetGuiName() const
	{
		return u8"Shader Test";
	}

	ImGuiWindowFlags ShaderTestWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}
}