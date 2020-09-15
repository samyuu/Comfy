#include "PJEFile.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "IO/Stream/Manipulator/StreamReader.h"

namespace Comfy::Studio::Editor
{
	namespace Legacy
	{
		std::unique_ptr<Chart> PJEFile::ToChart() const
		{
			auto chart = std::make_unique<Chart>();

			constexpr auto startOffsetAdjustment = TimeSpan::FromMilliseconds(20.0);
			chart->Properties.Song.Title = songTitle.data();
			chart->StartOffset = (songOffset == TimeSpan::Zero()) ? TimeSpan::Zero() : (songOffset + startOffsetAdjustment);

			// TODO: songEnd;
			chart->Duration = TimeSpan::Zero();

			std::vector<TempoChange> newTempoChanges;
			newTempoChanges.reserve(bpmChanges.size());
			for (const auto& bpmChange : bpmChanges)
				newTempoChanges.emplace_back(TimelineTick(bpmChange.Index), bpmChange.BPM, TempoChange::DefaultSignature);

			auto findMatchingTempoChange = [&](TimelineTick tick) -> TempoChange&
			{
				if (newTempoChanges.size() == 1)
					return newTempoChanges.front();

				for (size_t i = 0; i < newTempoChanges.size() - 1; i++)
				{
					auto& change = newTempoChanges[i];
					auto& nextChange = newTempoChanges[i + 1];

					if (change.Tick <= tick && nextChange.Tick > tick)
						return change;
				}

				return newTempoChanges.back();
			};

			for (const auto& pjeSigChange : sigChanges)
			{
				auto& matchingTempoChange = findMatchingTempoChange(TimelineTick(pjeSigChange.Index));
				auto signature = TimeSignature(pjeSigChange.Numerator, pjeSigChange.Denominator);

				if (matchingTempoChange.Tick == TimelineTick(pjeSigChange.Index))
					matchingTempoChange.Signature = signature;
				else
					newTempoChanges.emplace_back(TimelineTick(pjeSigChange.Index), matchingTempoChange.Tempo, signature);
			}

			std::vector<TimelineTarget> newTargets;
			newTargets.reserve(targets.size());
			for (const auto& pjeTarget : targets)
			{
				const auto targetTypeData = PJEFile::ConverTargetType(pjeTarget.Type);
				auto& newTarget = newTargets.emplace_back(TimelineTick(pjeTarget.Index), targetTypeData.Type);
				newTarget.Flags.HasProperties = true;
				newTarget.Flags.IsHold = targetTypeData.IsHold;
				newTarget.Flags.IsChain = targetTypeData.IsChain;
				newTarget.Flags.IsChance = targetTypeData.IsChance;
				newTarget.Properties.Position = pjeTarget.Position;
				newTarget.Properties.Angle = pjeTarget.Angle;
				newTarget.Properties.Frequency = static_cast<decltype(TargetProperties::Frequency)>(pjeTarget.Frequency);
				newTarget.Properties.Amplitude = pjeTarget.Amplitude;
				newTarget.Properties.Distance = pjeTarget.Distance;
			}

			chart->TempoMap = std::move(newTempoChanges);
			chart->Targets = std::move(newTargets);

			return chart;
		}

		std::string PJEFile::TryFindSongFilePath(std::string_view chartFilePath) const
		{
			const auto chartDirectory = IO::Path::GetDirectoryName(chartFilePath);
			const auto filePathNoExtension = IO::Path::Combine(chartDirectory, IO::Path::GetFileName(songTitle.data(), false));
			
			std::string combinedPath;
			combinedPath.reserve(filePathNoExtension.size() + (5 + 4));

			for (const auto extension : std::array { ".flac", ".ogg", ".mp3", ".wav" })
			{
				combinedPath.clear();
				combinedPath += filePathNoExtension;
				combinedPath += extension;

				if (IO::File::Exists(combinedPath))
					return combinedPath;

				combinedPath += ".lnk";
				if (IO::File::Exists(combinedPath))
					return IO::Shell::ResolveFileLink(combinedPath);
			}

			return "";
		}

		IO::StreamResult PJEFile::Read(IO::StreamReader& reader)
		{
			const auto magic = reader.ReadU32_BE();
			version = reader.ReadU32();
			if (magic != 'PJAE' || version > 0x4)
				return IO::StreamResult::BadFormat;

			const auto keepo = reader.ReadU32_BE();
			if (keepo != 'NOP\0')
				return IO::StreamResult::BadFormat;

			beatsPerBar = reader.ReadU32();
			if (beatsPerBar != 192)
				return IO::StreamResult::BadFormat;

			if (version == 0x1)
				reader.Skip(FileAddr(48));
			else
				reader.Skip(FileAddr(32));

			if (version > 0x1)
			{
				buttonSound = reader.ReadI16();
				slideSound = reader.ReadI16();
				chainSound = reader.ReadI16();
				touchSound = reader.ReadI16();
				pvID = reader.ReadU32();
				difficulty = reader.ReadU8();
				pvLevel = reader.ReadU8();
				reader.Skip(FileAddr(2));
			}

			const auto songIdentifier = reader.ReadU32_BE();
			if (songIdentifier != 'SNG\0')
				return IO::StreamResult::BadFormat;

			songOffset = TimeSpan::FromMilliseconds(reader.ReadF32());
			videoOffset = (version >= 0x3) ? TimeSpan::FromMilliseconds(reader.ReadF32()) : TimeSpan::Zero();

			songEnd.Bar = reader.ReadI32();
			songEnd.Beat = reader.ReadI32();

			reader.ReadBuffer(songTitle.data(), sizeof(songTitle));
			const auto songEndIdentifier = reader.ReadU32_BE();
			if (songEndIdentifier != 'END\0')
				return IO::StreamResult::BadFormat;

			const auto bpmIdentifier = reader.ReadU32_BE();
			if (bpmIdentifier != 'BPM\0')
				return IO::StreamResult::BadFormat;

			const auto bpmCount = reader.ReadU32();
			if (bpmCount < 1)
				return IO::StreamResult::BadCount;

			bpmChanges.reserve(bpmCount);
			for (size_t i = 0; i < bpmCount; i++)
			{
				auto& bpmChange = bpmChanges.emplace_back();
				if (version == 0x1)
				{
					bpmChange.BPM = reader.ReadF32();
					bpmChange.Index.Bar = reader.ReadI32();
					bpmChange.Index.Beat = reader.ReadI32();
				}
				else
				{
					bpmChange.Index.Bar = reader.ReadI32();
					bpmChange.Index.Beat = reader.ReadI32();
					bpmChange.BPM = reader.ReadF32();
				}
			}
			const auto bpmEndIdentifier = reader.ReadU32_BE();
			if (bpmEndIdentifier != 'END\0')
				return IO::StreamResult::BadFormat;

			const auto sigIdentifier = reader.ReadU32_BE();
			if (sigIdentifier != 'SIG\0')
				return IO::StreamResult::BadFormat;

			const auto sigCount = reader.ReadU32();
			sigChanges.reserve(sigCount);
			for (size_t i = 0; i < sigCount; i++)
			{
				auto& sigChange = sigChanges.emplace_back();
				sigChange.Index.Bar = reader.ReadI32();
				sigChange.Index.Beat = reader.ReadI32();
				sigChange.Numerator = reader.ReadU32();
				sigChange.Denominator = reader.ReadU32();

				if (sigChange.Numerator == 0)
					sigChange.Numerator = TempoChange::DefaultSignature.Numerator;

				if (sigChange.Denominator == 0)
					sigChange.Denominator = TempoChange::DefaultSignature.Denominator;
			}
			const auto sigEndIdentifier = reader.ReadU32_BE();
			if (sigEndIdentifier != 'END\0')
				return IO::StreamResult::BadFormat;

			const auto targetIdentifier = reader.ReadU32_BE();
			if (targetIdentifier != 'TGT\0')
				return IO::StreamResult::BadFormat;

			const auto targetCount = reader.ReadU32();
			targets.reserve(targetCount);
			for (size_t i = 0; i < targetCount; i++)
			{
				auto& target = targets.emplace_back();
				target.Index.Bar = reader.ReadI32();
				target.Index.Beat = reader.ReadI32();
				target.Type = static_cast<TargetType>(reader.ReadU32());
				target.Position = reader.ReadV2();
				target.Angle = reader.ReadF32();
				target.Frequency = reader.ReadF32();
				target.Amplitude = reader.ReadF32();
				if (version > 0x1)
					target.Distance = reader.ReadF32();
				target.FlyTime = reader.ReadF32();
			}

			if (version == 0x1)
			{
				for (auto& target : targets)
				{
					const bool isSync = std::count_if(targets.begin(), targets.end(), [&](auto& t) { return (t.Index.Bar == target.Index.Bar) && (t.Index.Beat == target.Index.Beat); }) > 1;

					target.Frequency = isSync ? 0.0f : 2.0f;
					target.Amplitude = 500.0f;
					target.Distance = 1200.0f;
				}
			}

			const auto targetEndIdentifier = reader.ReadU32_BE();
			if (targetEndIdentifier != 'END\0')
				return IO::StreamResult::BadFormat;

			const auto eof = reader.ReadU32_BE();
			if (eof != 'EOF\0')
				return IO::StreamResult::BadFormat;

			return IO::StreamResult::Success;
		}

		PJEFile::ButtonTypeData PJEFile::ConverTargetType(TargetType type)
		{
			switch (type)
			{
			case TargetType::Triangle: return { ButtonType::Triangle, 0, 0, 0 };
			case TargetType::Circle: return { ButtonType::Circle, 0, 0, 0 };
			case TargetType::Cross: return { ButtonType::Cross, 0, 0, 0 };
			case TargetType::Square: return { ButtonType::Square, 0, 0, 0 };
			case TargetType::TriangleHold: return { ButtonType::Triangle, 1, 0, 0 };
			case TargetType::CircleHold: return { ButtonType::Circle, 1, 0, 0 };
			case TargetType::CrossHold: return { ButtonType::Cross, 1, 0, 0 };
			case TargetType::SquareHold: return { ButtonType::Square, 1, 0, 0 };
			case TargetType::SlideL: return { ButtonType::SlideL, 0, 0, 0 };
			case TargetType::SlideR: return { ButtonType::SlideR, 0, 0, 0 };
			case TargetType::SlideLChain: return { ButtonType::SlideL, 0, 1, 0 };
			case TargetType::SlideRChain: return { ButtonType::SlideR, 0, 1, 0 };
			case TargetType::TriangleChance: return { ButtonType::Triangle, 0, 0, 1 };
			case TargetType::CircleChance: return { ButtonType::Circle, 0, 0, 1 };
			case TargetType::CrossChance: return { ButtonType::Cross, 0, 0, 1 };
			case TargetType::SquareChance: return { ButtonType::Square, 0, 0, 1 };
			case TargetType::SlideLChance: return { ButtonType::SlideL, 0, 0, 1 };
			case TargetType::SlideRChance: return { ButtonType::SlideR, 0, 0, 1 };
			}
			return { ButtonType::Circle, 0, 0, 0 };
		}

		PJEFile::TimelineIndex::operator TimelineTick() const
		{
			constexpr auto beatTickScale = (static_cast<f64>(TimelineTick::TicksPerBeat) * 4.0) / 192.0;
			return TimelineTick::FromBars(Bar, 4) + TimelineTick::FromTicks(static_cast<i32>(static_cast<f64>(Beat) * beatTickScale));
		}
	}
}
