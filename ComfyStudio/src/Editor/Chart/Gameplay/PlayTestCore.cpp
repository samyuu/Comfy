#include "PlayTestCore.h"
#include "PlayTestWindow.h"

namespace Comfy::Studio::Editor
{
	struct PlayTestCore::Impl
	{
	public:
		Impl(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext)
			: window(window), context(context), sharedContext(sharedContext)
		{
		}

	public:
		void UpdateTick()
		{
			// TODO:
		}

		void OverlayGui()
		{
		}

		bool ExitRequestedThisFrame()
		{
			const bool result = exitRequestedThisFrame;
			exitRequestedThisFrame = false;
			return result;
		}

	private:
		TimeSpan GetPlaybackTime() const
		{
			return sharedContext.SongVoice->GetPosition() - sharedContext.Chart->StartOffset;
		}

		void SetPlaybackTime(TimeSpan value)
		{
			sharedContext.SongVoice->SetPosition(value + sharedContext.Chart->StartOffset);
		}

	private:
		PlayTestWindow& window;
		PlayTestContext& context;
		PlayTestSharedContext& sharedContext;

		bool exitRequestedThisFrame = false;
	};

	PlayTestCore::PlayTestCore(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext)
		: impl(std::make_unique<Impl>(window, context, sharedContext))
	{
	}

	PlayTestCore::~PlayTestCore() = default;

	void PlayTestCore::UpdateTick()
	{
		impl->UpdateTick();
	}

	void PlayTestCore::OverlayGui()
	{
		impl->OverlayGui();
	}

	bool PlayTestCore::ExitRequestedThisFrame()
	{
		return impl->ExitRequestedThisFrame();
	}
}
