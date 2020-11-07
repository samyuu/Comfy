#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class TargetInspector : NonCopyable
	{
	public:
		TargetInspector(Undo::UndoManager& undoManager);
		~TargetInspector() = default;

	public:
		void Gui(Chart& chart);

	private:
		std::string_view FormatSelectedTargetsValueBuffer(const Chart& chart);

		void GuiSelectedTargets(Chart& chart);

		template <typename ValueGetter, typename ConditionGetter, typename OnChange>
		void BooleanGui(std::string_view label, ValueGetter valueGetter, ConditionGetter conditionGetter, OnChange onChange);

		void PropertyGui(Chart& chart, std::string_view label, TargetPropertyType property, f32 dragSpeed = 1.0f, bool isPropertyI32 = false, bool degreeUnits = false);

		i32 GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const;

	private:
		Undo::UndoManager& undoManager;

		std::array<char, 64> selectedTargetsValueBuffer;
		std::array<bool, TargetPropertyType_Count> propertyInputWidgetActiveStates = {};

		struct TargetViewProperties
		{
			TimelineTarget* Target;
			TargetProperties PropertiesOrPreset;

			TimelineTarget& operator*() { return *Target; }
			const TimelineTarget& operator*() const { return *Target; }
			TimelineTarget* operator->() { return Target; }
			const TimelineTarget* operator->() const { return Target; }
		};

		// NOTE: All of these are only valid between the begin and end of Gui()
		std::vector<TargetViewProperties> selectedTargets;

		TimelineTarget* frontSelectedTarget;
		TargetProperties frontSelectedProperties;
	};
}
