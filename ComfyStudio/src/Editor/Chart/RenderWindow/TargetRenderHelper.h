#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Render/Render.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Helper class to abstract away all of the aet and spr specific render interactions
	class TargetRenderHelper : NonCopyable
	{
	public:
		TargetRenderHelper();
		~TargetRenderHelper();

	public:
		void UpdateAsyncLoading();

		struct HUD
		{
			std::string_view SongName;

			bool IsPlayback;
			TimeSpan PlaybackTime;
			TimeSpan PlaybackTimeOnStart;
			TimeSpan Duration;
		};

		void DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
