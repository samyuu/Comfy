#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/SortedTargetList.h"
#include "Time/TimeSpan.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	enum class PlayTestExitType : u8;
	class PlayTestWindow;
	struct PlayTestContext;
	struct PlayTestSharedContext;

	enum class PlayTestSlidePositionType : u8
	{
		None,
		Left,
		Right,
		Count
	};

	struct PlayTestInputBinding
	{
		ButtonTypeFlags ButtonTypes;
		PlayTestSlidePositionType SlidePosition;
		Input::Binding InputSource;

		constexpr PlayTestInputBinding()
			: ButtonTypes(), SlidePosition(), InputSource()
		{
		}
		constexpr PlayTestInputBinding(ButtonTypeFlags buttonTypes, PlayTestSlidePositionType slidePosition, Input::KeyCode keyCode)
			: ButtonTypes(buttonTypes), SlidePosition(slidePosition), InputSource(keyCode)
		{
		}
		constexpr PlayTestInputBinding(ButtonTypeFlags buttonTypes, PlayTestSlidePositionType slidePosition, Input::Button button)
			: ButtonTypes(buttonTypes), SlidePosition(slidePosition), InputSource(button)
		{
		}
		constexpr bool IsEmpty() const
		{
			return (ButtonTypes == ButtonTypeFlags_None) || InputSource.IsEmpty();
		}
	};

	PlayTestInputBinding PlayTestBindingFromStorageString(std::string_view string);
	Input::FormatBuffer PlayTestBindingToStorageString(const PlayTestInputBinding& binding);

	class PlayTestCore : NonCopyable
	{
	public:
		PlayTestCore(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext);
		~PlayTestCore();

	public:
		void UpdateTick();
		void OverlayGui();
		PlayTestExitType GetAndClearExitRequestThisFrame();
		void Restart(TimeSpan startTime);

		bool GetAutoplayEnabled() const;
		void SetAutoplayEnabled(bool value);

		bool GetIsPlayback() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
