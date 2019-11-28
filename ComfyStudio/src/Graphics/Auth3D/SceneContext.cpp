#include "SceneContext.h"

namespace Graphics
{
	void SceneContext::Resize(ivec2 size)
	{
		RenderTarget.Resize(size);
	}
}
