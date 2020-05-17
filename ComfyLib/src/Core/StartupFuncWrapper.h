#pragma once

namespace Comfy
{
	template <typename Func>
	struct StartupFuncWrapper
	{
		StartupFuncWrapper(Func func) { func(); }
	};

	template <typename Func>
	StartupFuncWrapper<Func> WrapStartupFunc(Func func)
	{
		return { func };
	}
}
