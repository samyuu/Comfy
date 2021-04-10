#pragma once
#include "Types.h"
#include "InputBaseTypes.h"
#include "Time/TimeSpan.h"

namespace Comfy::Input
{
	using StandardControllerLayoutMappings = std::vector<StandardControllerLayoutMapping>;

	std::pair<const StandardControllerLayoutMapping*, size_t> GetKnownDS4LayoutMappingsView();

	void GlobalSystemInitialize(void* windowHandle);
	void GlobalSystemDispose(void* windowHandle);
	void GlobalSystemUpdateFrame(TimeSpan elapsedTime, bool hasApplicationFocus);
	void GlobalSystemRefreshDevices();
	void GlobalSystemSetExternalLayoutMappingsSource(const StandardControllerLayoutMappings* externalLayoutMappings);

	size_t GlobalSystemGetConnectedControllerCount();
	ControllerInfoView GlobalSystemGetConnectedControllerInfoAt(size_t index);

	template <typename Func>
	void GlobalSystemForEachConnectedController(Func func)
	{
		const size_t count = GlobalSystemGetConnectedControllerCount();
		for (size_t i = 0; i < count; i++)
			func(i, GlobalSystemGetConnectedControllerInfoAt(i));
	}

	TimeSpan GlobalSystemGetUpdateFrameProcessDuration();
}

namespace Comfy::Input
{
	FormatBuffer ControllerIDToString(const ControllerID& id);
	ControllerID ControllerIDFromString(std::string_view string);

	bool IsNativeButtonDown(const ControllerID& instanceID, NativeButton nativeButton);
	f32 GetNativeAxis(const ControllerID& instanceID, NativeAxis nativeAxis);

	bool AreAllModifiersDown(const KeyModifiers modifiers);
	bool WereAllModifiersDown(const KeyModifiers modifiers);
	bool AreAllModifiersUp(const KeyModifiers modifiers);
	bool WereAllModifiersUp(const KeyModifiers modifiers);
	bool AreOnlyModifiersDown(const KeyModifiers modifiers);
	bool WereOnlyModifiersDown(const KeyModifiers modifiers);

	bool IsKeyDown(const KeyCode keyCode);
	bool WasKeyDown(const KeyCode keyCode);
	bool IsKeyPressed(const KeyCode keyCode, bool repeat = true);
	bool IsKeyReleased(const KeyCode keyCode);

	bool IsButtonDown(const Button button);
	bool WasButtonDown(const Button button);
	bool IsButtonPressed(const Button button, bool repeat = true);
	bool IsButtonReleased(const Button button);

	f32 GetAxis(const Axis axis);
	vec2 GetStick(const Stick stick);

	bool IsDown(const Binding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool WasDown(const Binding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool IsPressed(const Binding& binding, bool repeat = true, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool IsReleased(const Binding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);

	bool IsAnyDown(const MultiBinding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool IsAnyPressed(const MultiBinding& binding, bool repeat = true, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool IsAnyReleased(const MultiBinding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);
	bool IsLastReleased(const MultiBinding& binding, ModifierBehavior behavior = ModifierBehavior_Strict);
}
