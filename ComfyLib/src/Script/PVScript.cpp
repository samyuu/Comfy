#include "PVScript.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include <numeric>

namespace Comfy
{
	void PVScriptBuilder::Add(TimeSpan time, const PVCommand& command, i32 branch)
	{
		// TODO: Correcetly handle branches
		assert(branch == 0);

		timedCommandGroups[PVCommandLayout::Time(time)].push_back(command);
	}

	std::vector<PVCommand> PVScriptBuilder::Create()
	{
		std::vector<PVCommand> outCommands;

		const size_t expectedCommandCount = std::accumulate(
			timedCommandGroups.begin(),
			timedCommandGroups.end(),
			size_t {},
			[&](size_t count, auto& group) { return (count + group.second.size() + 1); });

		outCommands.reserve(expectedCommandCount);

		for (const auto&[timeCommand, commandGroup] : timedCommandGroups)
		{
			outCommands.push_back(timeCommand);
			for (const auto& command : commandGroup)
				outCommands.push_back(command);
		}

		assert(outCommands.size() == expectedCommandCount);
		return outCommands;
	}

	namespace
	{
		constexpr bool ConstexprValidatePVCommandDescriptions()
		{
			for (u32 index = 0; index < EnumCount<PVCommandType>(); index++)
			{
				if (PVCommandInfoTable[index].Type != static_cast<PVCommandType>(index))
					return false;

				if (PVCommandInfoTable[index].ParamCount > MaxPVCommandParamCount)
					return false;
			}

			return true;
		}

		static_assert(ConstexprValidatePVCommandDescriptions());
	}

	void PVScript::Parse(const u8* buffer, size_t bufferSize)
	{
		if (bufferSize < sizeof(u32))
			return;

		Version = *reinterpret_cast<const u32*>(buffer);
		size_t commandCount = 0;

		const u32* commandsStart = reinterpret_cast<const u32*>(buffer + sizeof(u32));
		const u32* commandsEnd = reinterpret_cast<const u32*>(buffer + bufferSize);

		const u32* readHead = commandsStart;
		while (readHead < commandsEnd)
		{
			const auto commandType = *reinterpret_cast<const PVCommandType*>(readHead);
			const auto paramCount = GetPVCommandParamCount(commandType);

			readHead += (1 + paramCount);
			commandCount++;
		}

		readHead = commandsStart;

		Commands.reserve(commandCount);
		for (size_t i = 0; i < commandCount; i++)
		{
			const auto commandType = *reinterpret_cast<const PVCommandType*>(readHead);
			const auto paramCount = GetPVCommandParamCount(commandType);

			auto& command = Commands.emplace_back();
			command.Type = commandType;
			std::memcpy(command.Param.data(), (readHead + 1), paramCount * sizeof(i32));

			readHead += (1 + paramCount);
		}
	}

	IO::StreamResult PVScript::Write(IO::StreamWriter& writer)
	{
		writer.WriteU32(Version);
		for (const auto& command : Commands)
		{
			writer.WriteU32(static_cast<u32>(command.Type));
			for (size_t i = 0; i < command.ParamCount(); i++)
				writer.WriteU32(command.Param[i]);
		}

		if (Commands.empty() || Commands.back().Type != PVCommandType::End)
			writer.WriteU32(static_cast<u32>(PVCommandType::End));

		return IO::StreamResult::Success;
	}

	std::string PVScript::ToString() const
	{
		std::string out;
		out.reserve(Commands.size() * 32);

		for (const auto& command : Commands)
		{
			out += command.Name();

			out += '(';
			if (command.Type == PVCommandType::Time)
			{
				out += TimeSpan(command.View<PVCommandLayout::Time>()).FormatTime().data();
			}
			else
			{
				const auto paramCount = command.ParamCount();
				for (size_t i = 0; i < paramCount; i++)
				{
					char intStrBuffer[34];
					::_itoa_s(static_cast<i32>(command.Param[i]), intStrBuffer, 10);
					out += intStrBuffer;

					if (i + 1 != paramCount)
						out += ", ";
				}
			}
			out += ");";

			if (&command != &Commands.back())
				out += '\n';
		}

		return out;
	}
}
