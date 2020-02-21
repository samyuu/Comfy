#pragma once

namespace Graphics
{
	class IGraphicsResource : NonCopyable
	{
	protected:
		IGraphicsResource() = default;
		virtual ~IGraphicsResource() = default;

	protected:
	};
}
