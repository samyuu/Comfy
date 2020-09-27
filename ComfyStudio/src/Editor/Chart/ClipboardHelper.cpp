#include "ClipboardHelper.h"
#include "ImGui/Gui.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "System/Version/BuildVersion.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr std::string_view ClipboardVersionHash = std::string_view(BuildVersion::CommitHash, 8);
		constexpr std::string_view ChartEditorClipboardHeader = "#Comfy::Studio::ChartEditor Clipboard";

		// NOTE: Example clipboard data: (Only needs to be compatible with the version it was made with so it can easily change it the future)
		/*
			#Comfy::Studio::ChartEditor Clipboard 5e2d15a8
			Target { 192 0 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
			Target { 216 1 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
			Target { 240 2 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
			Target { 264 3 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
			Target { 288 4 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
			Target { 312 5 0 0 0 0 0.00 0.00 0.00 0.00 0.00 0.00 };
		*/
	}

	void ClipboardHelper::TimelineCopySelectedTargets(const std::vector<TimelineTarget>& targets)
	{
		constexpr size_t averageCharPerTargetLine = 128;

		buffer.reserve(targets.size() * averageCharPerTargetLine);
		buffer += ChartEditorClipboardHeader;
		buffer += " ";
		buffer += ClipboardVersionHash;

		for (const auto& target : targets)
		{
			const auto flags = target.Flags;
			const auto properties = target.Flags.HasProperties ? target.Properties : TargetProperties {};

			char b[2048];
			buffer += std::string_view(b, sprintf_s(b,
				"\nTarget { %d %d %d %d %d %d %.2f %.2f %.2f %.2f %.2f %.2f };",
				static_cast<i32>(target.Tick.Ticks()),
				static_cast<i32>(target.Type),
				static_cast<i32>(flags.HasProperties),
				static_cast<i32>(flags.IsHold),
				static_cast<i32>(flags.IsChain),
				static_cast<i32>(flags.IsChance),
				static_cast<f32>(properties.Position.x),
				static_cast<f32>(properties.Position.y),
				static_cast<f32>(properties.Angle),
				static_cast<f32>(properties.Frequency),
				static_cast<f32>(properties.Amplitude),
				static_cast<f32>(properties.Distance)
			));
		}

		SetClipboardText(buffer);
		buffer.clear();
	}

	std::optional<std::vector<TimelineTarget>> ClipboardHelper::TimelineTryGetPasteTargets()
	{
		const auto clipboardString = Util::Trim(GetClipboardText());
		if (clipboardString.empty())
			return {};

		const char* head = clipboardString.data();
		auto line = Util::Trim(Util::StringParsing::GetLineAdvanceToNextLine(head));

		if (!Util::StartsWith(line, ChartEditorClipboardHeader) || !Util::EndsWith(line, ClipboardVersionHash))
			return {};

		const auto lineCount = std::count(clipboardString.begin(), clipboardString.end(), '\n');
		std::vector<TimelineTarget> pasteTargets;
		pasteTargets.reserve(lineCount);

		do
		{
			line = Util::Trim(Util::StringParsing::GetLineAdvanceToNextLine(head));
			if (line.empty())
				break;

			if (Util::StartsWith(line, "Target {") && Util::EndsWith(line, "};"))
			{
				line = Util::Trim(line.substr(8, line.size() - 8 - 1));

				std::string_view word;
				auto advanceWord = [&] { line = (line.size() > word.size()) ? Util::Trim(line.substr(word.size())) : line.substr(0, 0); };
				auto parseI32 = [&] { return Util::StringParsing::ParseType<i32>(word = Util::StringParsing::GetWord(line.data())); };
				auto parseF32 = [&] { return Util::StringParsing::ParseType<f32>(word = Util::StringParsing::GetWord(line.data())); };

				auto& newTarget = pasteTargets.emplace_back();
				newTarget.Tick = TimelineTick::FromTicks(parseI32()); advanceWord();
				newTarget.Type = static_cast<ButtonType>(parseI32()); advanceWord();
				newTarget.Flags.HasProperties = parseI32(); advanceWord();
				newTarget.Flags.IsHold = parseI32(); advanceWord();
				newTarget.Flags.IsChain = parseI32(); advanceWord();
				newTarget.Flags.IsChance = parseI32(); advanceWord();
				newTarget.Properties.Position.x = parseF32(); advanceWord();
				newTarget.Properties.Position.y = parseF32(); advanceWord();
				newTarget.Properties.Angle = parseF32(); advanceWord();
				newTarget.Properties.Frequency = parseF32(); advanceWord();
				newTarget.Properties.Amplitude = parseF32(); advanceWord();
				newTarget.Properties.Distance = parseF32(); advanceWord();

				if (newTarget.Type >= ButtonType::Count)
					newTarget.Type = ButtonType::Triangle;
			}
		}
		while (head < (clipboardString.data() + clipboardString.size()));

		return pasteTargets;
	}

	std::string_view ClipboardHelper::GetClipboardText()
	{
		const auto clipboard = Gui::GetClipboardText();
		return (clipboard != nullptr) ? std::string_view(clipboard) : "";
	}

	void ClipboardHelper::SetClipboardText(const std::string& text)
	{
		Gui::SetClipboardText(text.c_str());
	}
}
