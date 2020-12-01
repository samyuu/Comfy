#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	class PlayTestWindow;
	struct PlayTestContext;
	struct PlayTestSharedContext;

	class PlayTestCore : NonCopyable
	{
	public:
		PlayTestCore(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext);
		~PlayTestCore();

	public:
		void UpdateTick();
		void OverlayGui();
		bool ExitRequestedThisFrame();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
