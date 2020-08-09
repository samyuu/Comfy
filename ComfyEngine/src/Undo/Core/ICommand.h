#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Undo
{
	enum class MergeResult
	{
		Failed,
		ValueUpdated,
	};

	class ICommand : NonCopyable
	{
	protected:
		// NOTE: Store new and old values as member fields and a reference to the data to be edited
		ICommand() = default;

	public:
		virtual ~ICommand() = default;

	public:
		// NOTE: Restore stored old value to the reference
		virtual void Undo() = 0;
		// NOTE: Apply stored new value to the reference
		virtual void Redo() = 0;

		// NOTE: The command parameter is garanteed to be safely castable to the type of the derived class.
		//		 Passed as non const reference to allow for move optimizations though it should not be mutated if the merge failed
		virtual MergeResult TryMerge(ICommand& commandToMerge) = 0;

		// NOTE: To be displayed in a GUI
		virtual std::string_view GetName() const = 0;
	};
}
