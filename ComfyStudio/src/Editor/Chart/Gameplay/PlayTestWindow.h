#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "PlayTestCore.h"
#include "Render/Render.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderHelper.h"
#include "Editor/Chart/RenderWindow/TargetRenderHelperEx.h"
#include "Editor/Common/SoundEffectManager.h"
#include "Editor/Common/ButtonSoundController.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/CheckerboardTexture.h"
#include "Input/Input.h"

// DEBUG:
#define COMFY_STUDIO_CHARTEDITOR_PLAYTEST_HOLDTEST 1

namespace Comfy::Studio::Editor
{
	struct PlayTestContext
	{
		Render::Camera2D Camera = {};
		std::unique_ptr<Render::RenderTarget2D> RenderTarget = Render::Renderer2D::CreateRenderTarget();
		TargetRenderHelperEx RenderHelperEx = {};

		struct ScoreData
		{
			i32 ComboCount;
			i32 ChainSlideScore;
			// TODO: Eventually (?)
			// i32 ScoreNumber;
			// i32 ScoreNumberRolling;

#if COMFY_STUDIO_CHARTEDITOR_PLAYTEST_HOLDTEST
			// TODO: Create hold state struct that keeps track of previous hold inputs to calculate score, max holds and animate the sync info display
			ButtonTypeFlags HoldTypes;
#endif
		} Score;
	};

	struct PlayTestSharedContext
	{
		// NOTE: All of these should safely outlive the duration of the parent object
		Render::Renderer2D* Renderer;
		TargetRenderHelper* RenderHelper;
		SoundEffectManager* SoundEffectManager;
		ButtonSoundController* ButtonSoundController;
		Audio::Voice* SongVoice;
		Chart* Chart;
	};

	class PlayTestWindow : NonCopyable
	{
	public:
		PlayTestWindow(PlayTestSharedContext sharedContext);
		~PlayTestWindow() = default;

	public:
		void ExclusiveGui();
		bool ExitRequestedThisFrame();

		void SetWorkingChart(Chart* chart);
		void Restart(TimeSpan startTime);

		bool GetAutoplayEnabled() const;
		void SetAutoplayEnabled(bool value);

		bool GetIsPlayback() const;

	public:
		vec2 WorldToScreenSpace(const vec2 worldSpace) const;
		vec2 ScreenToWorldSpace(const vec2 screenSpace) const;

	private:
		PlayTestContext context;
		PlayTestSharedContext sharedContext;

		PlayTestCore core = { *this, context, sharedContext };

		ImRect windowRect = {}, renderRegionRect = {};
		std::optional<Gui::CheckerboardTexture> windowBackgroundCheckerboard;

		bool exitRequestedThisFrame = false;
	};
}
