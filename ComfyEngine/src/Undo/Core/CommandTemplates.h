#pragma once
#include "ICommand.h"

namespace Comfy::Undo
{
	/* // NOTE: Example
	struct SharedPtrRefAccessor
	{
		static ValueType Get(RefType& ref);
		static void Set(RefType& ref, const ValueType& value);
		static constexpr std::string_view Name = "COMMAND_NAME";
	};
	*/

	template <typename RefType, typename ValueType, typename Accessor>
	class SharedPtrRefAccessorCommand : public ICommand
	{
	public:
		SharedPtrRefAccessorCommand(std::shared_ptr<RefType> ref, ValueType value)
			: ref(std::move(ref)), newValue(std::move(value)), oldValue(Accessor::Get(*ref))
		{
		}

	public:
		void Undo() override
		{
			Accessor::Set(*ref, oldValue);
		}

		void Redo() override
		{
			Accessor::Set(*ref, newValue);
		}

		MergeResult TryMerge(ICommand& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->ref.get() != ref.get())
				return MergeResult::Failed;

			newValue = std::move(other->newValue);
			return MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return Accessor::Name; }

	private:
		std::shared_ptr<RefType> ref;
		ValueType newValue, oldValue;
	};

#if 1 // DEBUG:
	struct DUMMY_COMMAND : public Undo::ICommand
	{
		template <typename... Args>
		DUMMY_COMMAND(Args... args) {}
		void Undo() override {}
		void Redo() override {}
		Undo::MergeResult TryMerge(ICommand& commandToMerge) override { return Undo::MergeResult::Failed; }
		std::string_view GetName() const override { return __FUNCTION__; }
	};
#endif
}
