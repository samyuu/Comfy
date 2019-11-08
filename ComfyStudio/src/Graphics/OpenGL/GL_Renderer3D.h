#pragma once
#include "Graphics/Camera.h"
#include "Graphics/OpenGL/GL_Shaders.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Graphics
{
	class GL_Renderer3D /* : public IRenderer */
	{
	public:
		void Initialize();
		void Begin(const PerspectiveCamera& camera);

		void Draw(const Obj* object, const vec3& position);

		void End();

	private:
		const PerspectiveCamera* perspectiveCamera;
		UniquePtr<GL_SimpleShader> simpleShader = nullptr;
	};
}