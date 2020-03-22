#pragma once
#include "Types.h"

#ifdef COMFY_D3D11
#include "Graphics/Direct3D/Renderer/D3D_Renderer2D.h"
#include "Graphics/Direct3D/Renderer/D3D_Renderer3D.h"
#endif

namespace Comfy::Graphics
{
	// TODO: Hide renderers behind proper interfaces
	
#ifdef COMFY_D3D11
	using GPU_Renderer2D = D3D_Renderer2D;
	using GPU_Renderer3D = D3D_Renderer3D;
#else
	class GPU_Renderer2D {};
	class GPU_Renderer3D {};
#endif
}
