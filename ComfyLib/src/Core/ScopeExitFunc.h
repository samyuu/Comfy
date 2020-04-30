#pragma once

namespace Comfy
{
	template <typename Func>
	struct ScopeExitFunc
	{
		ScopeExitFunc(Func&& func) : func(std::move(func)) {}
		~ScopeExitFunc() { func(); }
		Func func;
	};
}
