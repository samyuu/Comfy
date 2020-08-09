#pragma once
#include "ICommand.h"

namespace Comfy::Undo
{
	// NOTE: To be used as a static lookup table struct
	template <typename RefType, typename ValueType>
	struct EXAMPLE_SharedPtrRefAccessor
	{
		static ValueType Get(RefType& ref);
		static void Set(RefType& ref, const ValueType& value);
		static constexpr std::string_view Name = "Example Command";
	};

	template <typename RefType, typename ValueType, typename Accessor>
	class SharedPtrRefAccessorCommand : public ICommand
	{
	public:
		SharedPtrRefAccessorCommand(std::shared_ptr<RefType> ref, ValueType value)
			: reference(std::move(ref)), newValue(std::move(value)), oldValue(Accessor::Get(*reference))
		{
		}

	public:
		void Undo() override
		{
			Accessor::Set(*reference, oldValue);
		}

		void Redo() override
		{
			Accessor::Set(*reference, newValue);
		}

		MergeResult TryMerge(ICommand& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get())
				return MergeResult::Failed;

			newValue = std::move(other->newValue);
			return MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return Accessor::Name; }

	private:
		std::shared_ptr<RefType> reference;
		ValueType newValue, oldValue;
	};

	// DEBUG: Swallows all arguments without functionality.
	//		  Intended to be aliased to different names for testing commands that haven't yet been implemented
	class UnimplementedDummyCommand : public ICommand
	{
	public:
		template <typename... Args>
		UnimplementedDummyCommand(Args... args) {}

	public:
		void Undo() override {}
		void Redo() override {}
		MergeResult TryMerge(ICommand& commandToMerge) override { return MergeResult::Failed; }
		std::string_view GetName() const override { return "Unimplemented Dummy Command"; }
	};
}
