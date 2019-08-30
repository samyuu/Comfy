#pragma once
#include "Graphics/IRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Shader/Shader.h"
#include "FileSystem/Format/ObjSet.h"

namespace Graphics::Auth3D
{
	using namespace FileSystem;

	class Renderer3D : public IRenderer
	{
	public:
		void Initialize() override;
		void Begin(const PerspectiveCamera& camera);

		void Draw(const Obj* object, const vec3& position);

		void End();

	private:
		const PerspectiveCamera* camera;
		UniquePtr<SimpleShader> simpleShader;
	};
}