#include "ComfyStudioChartFile.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include "System/Version/BuildVersion.h"
#include "System/Version/BuildConfiguration.h"
#include "Misc/StringParseHelper.h"
#include <ctime>

namespace Comfy::Studio::Editor
{
	namespace ChartFileFormat
	{
		// NOTE: Increment major version for breaking changes and minor version for backwards and forward compatible additions
		enum class Version : u16 { CurrentMajor = 1, CurrentMinor = 5, };
		enum class Endianness : u16 { Little = 'L', Big = 'B' };
		enum class PointerSize : u16 { Bit32 = 32, Bit64 = 64 };
		enum class HeaderFlags : u32 { None = 0xFFFFFFFF };

		constexpr std::array<u8, 4> Magic = { 'C', 'S', 'F', 'M' };
		constexpr std::array<char, 8> Encoding = { "UTF-8" };

		struct HeaderData
		{
			std::array<u8, 4> FileMagic;
			Version MajorVersion;
			Version MinorVersion;
			Endianness Endianness;
			PointerSize PointerSize;
			HeaderFlags Flags;
			__time64_t CreationTime;
			std::array<char, 8> CharacterEncoding;
			std::array<u32, 8> Reserved;
		};

		static_assert(sizeof(HeaderData) == 64);

#ifndef _WIN32
		static_assert(false);
#endif /* _WIN32 */

#ifndef _WIN64
		static_assert(false);
#endif /* _WIN64 */

		constexpr std::array<std::string_view, 10> CreatorInfoStrings =
		{
			"Comfy Studio",
			"win32",
			"x64",
			BuildVersion::Author,
			BuildVersion::CommitHash,
			BuildVersion::CommitTime,
			BuildVersion::CommitNumber,
			BuildVersion::Branch,
			BuildVersion::CompileTime,
			BuildConfiguration::Debug ? "Debug" : BuildConfiguration::Release ? "Release" : "Unknown",
		};

		constexpr std::string_view SectionIDError = "Error";
		constexpr std::string_view SectionIDMetadata = "Metadata";
		constexpr std::string_view SectionIDMetadataSongFileName = "Song File Name";
		constexpr std::string_view SectionIDMetadataSongTitle = "Song Title";
		constexpr std::string_view SectionIDMetadataSongTitleReading = "Song Title Reading";
		constexpr std::string_view SectionIDMetadataArtist = "Artist";
		constexpr std::string_view SectionIDMetadataAlbum = "Album";
		constexpr std::string_view SectionIDMetadataLyricist = "Lyricist";
		constexpr std::string_view SectionIDMetadataArranger = "Arranger";
		constexpr std::string_view SectionIDMetadataTrackNumber = "Track Number";
		constexpr std::string_view SectionIDMetadataDiskNumber = "Disk Number";
		constexpr std::string_view SectionIDMetadataCreatorName = "Creator Name";
		constexpr std::string_view SectionIDMetadataCreatorComment = "Creator Comment";
		constexpr std::string_view SectionIDMetadataExtraInfoKeyFmt = "Extra Info Key %zu";
		constexpr std::string_view SectionIDMetadataExtraInfoValueFmt = "Extra Info Value %zu";
		constexpr std::string_view SectionIDMetadataImageCoverFileName = "Cover File Name";
		constexpr std::string_view SectionIDMetadataImageLogoFileName = "Logo File Name";
		constexpr std::string_view SectionIDMetadataImageBackgroundFileName = "Background File Name";

		constexpr std::string_view SectionIDChart = "Chart";
		constexpr std::string_view SectionIDChartSectionIDScale = "Scale";
		constexpr std::string_view SectionIDChartSectionIDTime = "Time";
		constexpr std::string_view SectionIDChartSectionIDTimeSongOffset = "Song Offset";
		constexpr std::string_view SectionIDChartSectionIDTimeDuration = "Duration";
		constexpr std::string_view SectionIDChartSectionIDTimeSongPreviewStart = "Song Preview Start";
		constexpr std::string_view SectionIDChartSectionIDTimeSongPreviewDuration = "Song Preview Duration";
		constexpr std::string_view SectionIDChartSectionIDTargets = "Targets";
		constexpr std::string_view SectionIDChartSectionIDTempoMap = "Tempo Map";
		constexpr std::string_view SectionIDChartSectionIDButtonSounds = "Button Sounds";
		constexpr std::string_view SectionIDChartSectionIDDifficulty = "Difficulty";

		constexpr std::string_view SectionIDDebug = "Debug";

		struct TargetField
		{
			std::string_view Name;
			size_t ByteSize;
			void(*ReadFunc)(IO::StreamReader&, TimelineTarget&);
			void(*WriteFunc)(IO::StreamWriter&, const TimelineTarget&);
		};

		constexpr std::array<TargetField, 11> TargetFields =
		{
			TargetField { "Tick", sizeof(i32), [](IO::StreamReader& r, TimelineTarget& t) { t.Tick = BeatTick(r.ReadI32()); }, [](IO::StreamWriter& writer, const TimelineTarget& target) { writer.WriteI32(target.Tick.Ticks()); } },
			TargetField { "Type", sizeof(u8), [](IO::StreamReader& r, TimelineTarget& t) { t.Type = static_cast<ButtonType>(r.ReadU8()); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteU8(static_cast<u8>(t.Type)); } },
			TargetField { "Properties", sizeof(u8), [](IO::StreamReader& r, TimelineTarget& t) { t.Flags.HasProperties = r.ReadU8(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteU8(t.Flags.HasProperties); } },
			TargetField { "Hold", sizeof(u8), [](IO::StreamReader& r, TimelineTarget& t) { t.Flags.IsHold = r.ReadU8(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteU8(t.Flags.IsHold); } },
			TargetField { "Chain", sizeof(u8), [](IO::StreamReader& r, TimelineTarget& t) { t.Flags.IsChain = r.ReadU8(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteU8(t.Flags.IsChain); } },
			TargetField { "Chance", sizeof(u8), [](IO::StreamReader& r, TimelineTarget& t) { t.Flags.IsChance = r.ReadU8(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteU8(t.Flags.IsChance); } },
			TargetField { "Position", sizeof(vec2), [](IO::StreamReader& r, TimelineTarget& t) { t.Properties.Position = r.ReadV2(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteF32(t.Properties.Position.x); w.WriteF32(t.Properties.Position.y); } },
			TargetField { "Angle", sizeof(f32), [](IO::StreamReader& r, TimelineTarget& t) { t.Properties.Angle = r.ReadF32(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteF32(t.Properties.Angle); } },
			TargetField { "Frequency", sizeof(f32), [](IO::StreamReader& r, TimelineTarget& t) { t.Properties.Frequency = r.ReadF32(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteF32(t.Properties.Frequency); } },
			TargetField { "Amplitude", sizeof(f32), [](IO::StreamReader& r, TimelineTarget& t) { t.Properties.Amplitude = r.ReadF32(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteF32(t.Properties.Amplitude); } },
			TargetField { "Distance", sizeof(f32), [](IO::StreamReader& r, TimelineTarget& t) { t.Properties.Distance = r.ReadF32(); }, [](IO::StreamWriter& w, const TimelineTarget& t) { w.WriteF32(t.Properties.Distance); } },
		};

		struct TempoField
		{
			std::string_view Name;
			size_t ByteSize;
			void(*ReadFunc)(IO::StreamReader&, TempoChange&);
			void(*WriteFunc)(IO::StreamWriter&, const TempoChange&);
		};

		constexpr std::array<TempoField, 3> TempoMapFields =
		{
			TempoField { "Tick", sizeof(i32), [](IO::StreamReader& r, TempoChange& t) { t.Tick = BeatTick(r.ReadI32()); }, [](IO::StreamWriter& w, const TempoChange& t) { w.WriteI32(t.Tick.Ticks()); } },
			TempoField { "Tempo", sizeof(f32), [](IO::StreamReader& r, TempoChange& t) { t.Tempo = Tempo(r.ReadF32()); },[](IO::StreamWriter& w, const TempoChange& t) { w.WriteF32(t.Tempo.BeatsPerMinute); } },
			TempoField { "Time Signature", sizeof(i16) * 2, [](IO::StreamReader& r, TempoChange& t) { t.Signature.Numerator = r.ReadI16(); t.Signature.Denominator = r.ReadI16(); },[](IO::StreamWriter& w, const TempoChange& t) { w.WriteI16(t.Signature.Numerator); w.WriteI16(t.Signature.Denominator); } },
		};
	}

	ComfyStudioChartFile::ComfyStudioChartFile(const Chart& sourceChart)
	{
		FromChart(sourceChart);
	}

	std::unique_ptr<Chart> ComfyStudioChartFile::MoveToChart()
	{
		using namespace ChartFileFormat;

		auto outChart = std::make_unique<Chart>();

		auto findMetadataStr = [this](std::string_view key, std::string& outValue) { outValue = metadata.Find(key); };
		auto findMetadataI32 = [this](std::string_view key, i32& outValue) { outValue = Util::StringParsing::ParseType<i32>(metadata.Find(key)); };
		findMetadataStr(SectionIDMetadataSongFileName, outChart->SongFileName);
		findMetadataStr(SectionIDMetadataSongTitle, outChart->Properties.Song.Title);
		findMetadataStr(SectionIDMetadataSongTitleReading, outChart->Properties.Song.TitleReading);
		findMetadataStr(SectionIDMetadataArtist, outChart->Properties.Song.Artist);
		findMetadataStr(SectionIDMetadataAlbum, outChart->Properties.Song.Album);
		findMetadataStr(SectionIDMetadataLyricist, outChart->Properties.Song.Lyricist);
		findMetadataStr(SectionIDMetadataArranger, outChart->Properties.Song.Arranger);
		findMetadataI32(SectionIDMetadataTrackNumber, outChart->Properties.Song.TrackNumber);
		findMetadataI32(SectionIDMetadataDiskNumber, outChart->Properties.Song.DiskNumber);
		findMetadataStr(SectionIDMetadataCreatorName, outChart->Properties.Creator.Name);
		findMetadataStr(SectionIDMetadataCreatorComment, outChart->Properties.Creator.Comment);
		findMetadataStr(SectionIDMetadataImageCoverFileName, outChart->Properties.Image.CoverFileName);
		findMetadataStr(SectionIDMetadataImageLogoFileName, outChart->Properties.Image.LogoFileName);
		findMetadataStr(SectionIDMetadataImageBackgroundFileName, outChart->Properties.Image.BackgroundFileName);

		for (size_t i = 0; i < outChart->Properties.Song.ExtraInfo.size(); i++)
		{
			char buffer[32];
			findMetadataStr(std::string_view(buffer, sprintf_s(buffer, SectionIDMetadataExtraInfoKeyFmt.data(), i)), outChart->Properties.Song.ExtraInfo[i].Key);
			findMetadataStr(std::string_view(buffer, sprintf_s(buffer, SectionIDMetadataExtraInfoValueFmt.data(), i)), outChart->Properties.Song.ExtraInfo[i].Value);
		}

		outChart->StartOffset = chart.Time.SongOffset;
		outChart->Duration = chart.Time.Duration;
		outChart->Properties.SongPreview.StartTime = chart.Time.SongPreviewStart;
		outChart->Properties.SongPreview.Duration = chart.Time.SongPreviewDuration;

		outChart->TempoMap = std::move(chart.TempoMap);
		outChart->Targets = std::move(chart.Targets);

		outChart->Properties.ButtonSound.ButtonID = chart.ButtonSound.ButtonID;
		outChart->Properties.ButtonSound.SlideID = chart.ButtonSound.SlideID;
		outChart->Properties.ButtonSound.ChainSlideID = chart.ButtonSound.ChainSlideID;
		outChart->Properties.ButtonSound.SliderTouchID = chart.ButtonSound.SliderTouchID;

		const bool isOriginalVersion = (chart.Difficulty.Version == ChartData::DifficultyVersion::Original);
		switch (chart.Difficulty.Type)
		{
		case ChartData::DifficultyType::Easy: { outChart->Properties.Difficulty.Type = Difficulty::Easy; break; }
		case ChartData::DifficultyType::Normal: { outChart->Properties.Difficulty.Type = Difficulty::Normal; break; }
		case ChartData::DifficultyType::Hard: { outChart->Properties.Difficulty.Type = Difficulty::Hard; break; }
		case ChartData::DifficultyType::Extreme: { outChart->Properties.Difficulty.Type = isOriginalVersion ? Difficulty::Extreme : Difficulty::ExExtreme; break; }
		}
		outChart->Properties.Difficulty.Level = static_cast<DifficultyLevel>(static_cast<u8>(chart.Difficulty.LevelWhole) * 2 + (chart.Difficulty.LevelFraction == ChartData::DifficultyLevelFraction::Half));

		return outChart;
	}

	IO::StreamResult ComfyStudioChartFile::Read(IO::StreamReader& reader)
	{
		using namespace ChartFileFormat;

		reader.SetPointerMode(IO::PtrMode::Mode64Bit);
		reader.SetEndianness(IO::Endianness::Little);

		if (static_cast<size_t>(reader.GetLength()) < sizeof(HeaderData))
			return IO::StreamResult::BadFormat;

		static_assert(IO::Endianness::Native == IO::Endianness::Little);
		HeaderData header;
		reader.ReadBuffer(&header, sizeof(header));

		if (header.FileMagic != Magic)
			return IO::StreamResult::BadFormat;

		if (header.MajorVersion > Version::CurrentMajor)
			return IO::StreamResult::BadFormat;

		if (header.Endianness != Endianness::Little)
			return IO::StreamResult::BadFormat;

		if (header.PointerSize != PointerSize::Bit64)
			return IO::StreamResult::BadFormat;

		if (header.CharacterEncoding != Encoding)
			return IO::StreamResult::BadFormat;

		const auto creatorInfoAddress = reader.GetPosition();
		const auto creatorInfoByteSize = reader.ReadSize();
		// TODO: Parse and store creator info
		reader.Seek(creatorInfoAddress + static_cast<FileAddr>(creatorInfoByteSize));

		const auto sectionCount = reader.ReadSize();
		const auto sectionsOffset = reader.ReadPtr();
		reader.ReadU64();
		reader.ReadU64();

		if (sectionCount < 1)
			return IO::StreamResult::BadCount;

		if (!reader.IsValidPointer(sectionsOffset))
			return IO::StreamResult::BadPointer;

		reader.ReadAt(sectionsOffset, [&](IO::StreamReader& reader)
		{
			for (size_t sectionIndex = 0; sectionIndex < sectionCount; sectionIndex++)
			{
				const auto sectionNameID = reader.ReadStrPtrOffsetAware();
				const auto sectionOffset = reader.ReadPtr();
				reader.ReadU64();
				reader.ReadU64();

				if (!reader.IsValidPointer(sectionOffset))
					continue;

				reader.ReadAt(sectionOffset, [&](IO::StreamReader& reader)
				{
					if (sectionNameID == SectionIDMetadata)
					{
						const auto entryCount = reader.ReadSize();
						const auto entriesOffset = reader.ReadPtr();
						reader.ReadU64();
						reader.ReadU64();

						if (!reader.IsValidPointer(entriesOffset))
							return;

						reader.ReadAt(entriesOffset, [&](IO::StreamReader& reader)
						{
							metadata.Items.reserve(entryCount);
							for (size_t entryIndex = 0; entryIndex < entryCount; entryIndex++)
							{
								auto key = reader.ReadStrPtrOffsetAware();
								auto value = reader.ReadStrPtrOffsetAware();
								reader.ReadU64();
								reader.ReadU64();

								metadata.Add(std::move(key), std::move(value));
							}
						});
					}
					else if (sectionNameID == SectionIDChart)
					{
						const auto chartSectionCount = reader.ReadSize();
						const auto chartSectionsOffset = reader.ReadPtr();
						reader.ReadU64();
						reader.ReadU64();

						if (!reader.IsValidPointer(chartSectionsOffset))
							return;

						reader.ReadAt(chartSectionsOffset, [&](IO::StreamReader& reader)
						{
							for (size_t chartSectionIndex = 0; chartSectionIndex < chartSectionCount; chartSectionIndex++)
							{
								const auto chartSectionNameID = reader.ReadStrPtrOffsetAware();
								const auto chartSectionOffset = reader.ReadPtr();
								reader.ReadU64();
								reader.ReadU64();

								if (!reader.IsValidPointer(chartSectionOffset))
									return;

								reader.ReadAt(chartSectionOffset, [&](IO::StreamReader& reader)
								{
									if (chartSectionNameID == SectionIDChartSectionIDScale)
									{
										const auto buttonTypeNameCount = reader.ReadSize();
										const auto buttonTypeNamesOffset = reader.ReadPtr();
										chart.Scale.TicksPerBeat = reader.ReadI32();
										reader.ReadI32();
										chart.Scale.PlacementAreaSize = reader.ReadV2();
										chart.Scale.FullAngleRotation = reader.ReadF32();
										if (chart.Scale.FullAngleRotation == 0.0f)
											chart.Scale.FullAngleRotation = 360.0f;
										reader.ReadU32();
										reader.ReadU64();
										reader.ReadU64();
										reader.ReadU64();

										if (reader.IsValidPointer(buttonTypeNamesOffset))
										{
											reader.ReadAt(buttonTypeNamesOffset, [&](IO::StreamReader& reader)
											{
												chart.Scale.ButtonTypeNames.reserve(buttonTypeNameCount);
												for (size_t i = 0; i < buttonTypeNameCount; i++)
													chart.Scale.ButtonTypeNames.push_back(std::move(reader.ReadStrPtrOffsetAware()));
											});
										}
									}
									else if (chartSectionNameID == SectionIDChartSectionIDTime)
									{
										const auto entryCount = reader.ReadSize();
										const auto entriesOffset = reader.ReadPtr();
										reader.ReadU64();
										reader.ReadU64();

										if (reader.IsValidPointer(entriesOffset))
										{
											reader.ReadAt(entriesOffset, [&](IO::StreamReader& reader)
											{
												for (size_t entryIndex = 0; entryIndex < entryCount; entryIndex++)
												{
													const auto key = reader.ReadStrPtrOffsetAware();
													const auto value = TimeSpan::FromSeconds(reader.ReadF64());
													reader.ReadU64();
													reader.ReadU64();

													if (key == SectionIDChartSectionIDTimeSongOffset)
														chart.Time.SongOffset = value;
													else if (key == SectionIDChartSectionIDTimeDuration)
														chart.Time.Duration = value;
													else if (key == SectionIDChartSectionIDTimeSongPreviewStart)
														chart.Time.SongPreviewStart = value;
													else if (key == SectionIDChartSectionIDTimeSongPreviewDuration)
														chart.Time.SongPreviewDuration = value;
												}
											});
										}
									}
									else if (chartSectionNameID == SectionIDChartSectionIDTargets)
									{
										const auto targetCount = reader.ReadSize();
										const auto fieldCount = reader.ReadSize();
										const auto fieldsOffset = reader.ReadPtr();
										reader.ReadU64();

										if (reader.IsValidPointer(fieldsOffset))
										{
											reader.ReadAt(fieldsOffset, [&](IO::StreamReader& reader)
											{
												chart.Targets.resize(targetCount);
												for (size_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
												{
													const auto nameID = reader.ReadStrPtrOffsetAware();
													const auto fieldByteSize = reader.ReadSize();
													const auto fieldArraySize = reader.ReadSize();
													const auto fieldOffset = reader.ReadPtr();
													reader.ReadU64();
													reader.ReadU64();

													if (reader.IsValidPointer(fieldOffset))
													{
														reader.ReadAt(fieldOffset, [&](IO::StreamReader& reader)
														{
															for (const auto& field : TargetFields)
															{
																if (nameID == field.Name)
																{
																	for (size_t i = 0; i < targetCount; i++)
																		field.ReadFunc(reader, chart.Targets[i]);
																	break;
																}
															}
														});
													}
												}
											});
										}
									}
									else if (chartSectionNameID == SectionIDChartSectionIDTempoMap)
									{
										const auto tempoChangeCount = reader.ReadSize();
										const auto fieldCount = reader.ReadSize();
										const auto fieldsOffset = reader.ReadPtr();
										reader.ReadU64();

										if (reader.IsValidPointer(fieldsOffset))
										{
											reader.ReadAt(fieldsOffset, [&](IO::StreamReader& reader)
											{
												chart.TempoMap.resize(tempoChangeCount);
												for (size_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
												{
													const auto nameID = reader.ReadStrPtrOffsetAware();
													const auto fieldByteSize = reader.ReadSize();
													const auto fieldArraySize = reader.ReadSize();
													const auto fieldOffset = reader.ReadPtr();
													reader.ReadU64();
													reader.ReadU64();

													if (reader.IsValidPointer(fieldOffset))
													{
														reader.ReadAt(fieldOffset, [&](IO::StreamReader& reader)
														{
															for (const auto& field : TempoMapFields)
															{
																if (nameID == field.Name)
																{
																	for (size_t i = 0; i < tempoChangeCount; i++)
																		field.ReadFunc(reader, chart.TempoMap[i]);
																	break;
																}
															}
														});
													}
												}
											});
										}
									}
									else if (chartSectionNameID == SectionIDChartSectionIDButtonSounds)
									{
										const auto buttonIDCount = reader.ReadSize();
										const auto buttonIDsOffset = reader.ReadPtr();
										reader.ReadU64();
										reader.ReadU64();

										if (reader.IsValidPointer(buttonIDsOffset) && buttonIDCount >= 4)
										{
											reader.ReadAt(buttonIDsOffset, [&](IO::StreamReader& reader)
											{
												chart.ButtonSound.ButtonID = reader.ReadU32();
												chart.ButtonSound.SlideID = reader.ReadU32();
												chart.ButtonSound.ChainSlideID = reader.ReadU32();
												chart.ButtonSound.SliderTouchID = reader.ReadU32();
												reader.ReadU64();
												reader.ReadU64();
											});
										}
									}
									else if (chartSectionNameID == SectionIDChartSectionIDDifficulty)
									{
										chart.Difficulty.Type = static_cast<ChartData::DifficultyType>(reader.ReadU8());
										chart.Difficulty.Version = static_cast<ChartData::DifficultyVersion>(reader.ReadU8());
										chart.Difficulty.LevelWhole = static_cast<ChartData::DifficultyLevelWhole>(reader.ReadU8());
										chart.Difficulty.LevelFraction = static_cast<ChartData::DifficultyLevelFraction>(reader.ReadU8());
										reader.ReadU64();
										reader.ReadU64();
									}
								});
							}
						});
					}
				});
			}
		});

		return IO::StreamResult::Success;
	}

	IO::StreamResult ComfyStudioChartFile::Write(IO::StreamWriter& writer)
	{
		using namespace ChartFileFormat;

		HeaderData header;
		header.FileMagic = Magic;
		header.MajorVersion = Version::CurrentMajor;
		header.MinorVersion = Version::CurrentMinor;
		header.Endianness = Endianness::Little;
		header.PointerSize = PointerSize::Bit64;
		header.Flags = HeaderFlags::None;
		header.CreationTime = time(nullptr);
		header.CharacterEncoding = Encoding;
		header.Reserved = {};

		writer.SetPointerMode(IO::PtrMode::Mode64Bit);
		writer.SetEndianness(IO::Endianness::Little);

		static_assert(IO::Endianness::Native == IO::Endianness::Little);
		writer.WriteBuffer(&header, sizeof(header));

		static constexpr auto creatorInfoByteSize = sizeof(u64) + (CreatorInfoStrings.size() * sizeof(u64)) + sizeof(u64);
		writer.WriteSize(creatorInfoByteSize);
		for (const auto& info : CreatorInfoStrings)
			writer.WriteStrPtr(info);
		writer.WriteU64(0);

		static constexpr auto sectionCount = 3;
		writer.WriteSize(sectionCount);
		writer.WriteFuncPtr([&](IO::StreamWriter& writer)
		{
			for (size_t sectionIndex = 0; sectionIndex < sectionCount; sectionIndex++)
			{
				if (sectionIndex == 0)
				{
					writer.WriteStrPtr(SectionIDMetadata);
					writer.WriteFuncPtr([&](IO::StreamWriter& writer)
					{
						writer.WriteSize(metadata.Items.size());
						writer.WriteFuncPtr([&](IO::StreamWriter& writer)
						{
							for (const auto& item : metadata.Items)
							{
								writer.WriteStrPtr(item.Key);
								writer.WriteStrPtr(item.Value);
								writer.WriteU64(0);
								writer.WriteU64(0);
							}
							writer.WriteAlignmentPadding(16);
						});
						writer.WriteU64(0);
						writer.WriteU64(0);
						writer.WriteAlignmentPadding(16);
					});
					writer.WriteU64(0);
					writer.WriteU64(0);
				}
				else if (sectionIndex == 1)
				{
					writer.WriteStrPtr(SectionIDChart);
					writer.WriteFuncPtr([&](IO::StreamWriter& writer)
					{
						static constexpr auto chartSectionCount = 6;
						writer.WriteSize(chartSectionCount);
						writer.WriteFuncPtr([&](IO::StreamWriter& writer)
						{
							for (size_t chartSectionIndex = 0; chartSectionIndex < chartSectionCount; chartSectionIndex++)
							{
								if (chartSectionIndex == 0)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDScale);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										writer.WriteSize(chart.Scale.ButtonTypeNames.size());
										writer.WriteFuncPtr([&](IO::StreamWriter& writer)
										{
											for (const auto& name : chart.Scale.ButtonTypeNames)
												writer.WriteStrPtr(name);
											writer.WriteAlignmentPadding(16);
										});
										writer.WriteI32(chart.Scale.TicksPerBeat);
										writer.WriteI32(0);
										writer.WriteF32(chart.Scale.PlacementAreaSize.x);
										writer.WriteF32(chart.Scale.PlacementAreaSize.y);
										writer.WriteF32(chart.Scale.FullAngleRotation);
										writer.WriteU32(0);
										writer.WriteU64(0);
										writer.WriteU64(0);
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else if (chartSectionIndex == 1)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDTime);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										const auto timeEntries = std::array
										{
											std::make_pair(SectionIDChartSectionIDTimeSongOffset, chart.Time.SongOffset),
											std::make_pair(SectionIDChartSectionIDTimeDuration, chart.Time.Duration),
											std::make_pair(SectionIDChartSectionIDTimeSongPreviewStart, chart.Time.SongPreviewStart),
											std::make_pair(SectionIDChartSectionIDTimeSongPreviewDuration, chart.Time.SongPreviewDuration),
										};

										writer.WriteSize(timeEntries.size());
										writer.WriteFuncPtr([&, timeEntries](IO::StreamWriter& writer)
										{
											for (const auto&[name, time] : timeEntries)
											{
												writer.WriteStrPtr(name);
												writer.WriteF64(time.TotalSeconds());
												writer.WriteU64(0);
												writer.WriteU64(0);
											}
											writer.WriteAlignmentPadding(16);
										});
										writer.WriteU64(0);
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else if (chartSectionIndex == 2)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDTargets);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										writer.WriteSize(chart.Targets.size());
										writer.WriteSize(TargetFields.size());
										writer.WriteFuncPtr([&](IO::StreamWriter& writer)
										{
											for (const auto& field : TargetFields)
											{
												// TODO: Store boolean flags in packed bit arrays
												writer.WriteStrPtr(field.Name);
												writer.WriteSize(field.ByteSize);
												writer.WriteSize(field.ByteSize * chart.Targets.size());
												writer.WriteFuncPtr([&](IO::StreamWriter& writer)
												{
													for (const auto& target : chart.Targets)
														field.WriteFunc(writer, target);
													writer.WriteAlignmentPadding(16);
												});
												writer.WriteU64(0);
												writer.WriteU64(0);
											}
											writer.WriteAlignmentPadding(16);
										});
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else if (chartSectionIndex == 3)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDTempoMap);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										writer.WriteSize(chart.TempoMap.size());
										writer.WriteSize(TempoMapFields.size());
										writer.WriteFuncPtr([&](IO::StreamWriter& writer)
										{
											for (const auto& field : TempoMapFields)
											{
												writer.WriteStrPtr(field.Name);
												writer.WriteSize(field.ByteSize);
												writer.WriteSize(field.ByteSize * chart.TempoMap.size());
												writer.WriteFuncPtr([&](IO::StreamWriter& writer)
												{
													for (const auto& tempoChange : chart.TempoMap)
														field.WriteFunc(writer, tempoChange);
													writer.WriteAlignmentPadding(16);
												});
												writer.WriteU64(0);
												writer.WriteU64(0);
											}
											writer.WriteAlignmentPadding(16);
										});
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else if (chartSectionIndex == 4)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDButtonSounds);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										constexpr auto buttonSoundIDCount = 4;
										writer.WriteSize(buttonSoundIDCount);
										writer.WriteFuncPtr([&](IO::StreamWriter& writer)
										{
											writer.WriteU32(chart.ButtonSound.ButtonID);
											writer.WriteU32(chart.ButtonSound.SlideID);
											writer.WriteU32(chart.ButtonSound.ChainSlideID);
											writer.WriteU32(chart.ButtonSound.SliderTouchID);
											writer.WriteU64(0);
											writer.WriteU64(0);
											writer.WriteAlignmentPadding(16);
										});
										writer.WriteU64(0);
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else if (chartSectionIndex == 5)
								{
									writer.WriteStrPtr(SectionIDChartSectionIDDifficulty);
									writer.WriteFuncPtr([&](IO::StreamWriter& writer)
									{
										writer.WriteU8(static_cast<u8>(chart.Difficulty.Type));
										writer.WriteU8(static_cast<u8>(chart.Difficulty.Version));
										writer.WriteU8(static_cast<u8>(chart.Difficulty.LevelWhole));
										writer.WriteU8(static_cast<u8>(chart.Difficulty.LevelFraction));
										writer.WriteU64(0);
										writer.WriteU64(0);
										writer.WriteAlignmentPadding(16);
									});
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
								else
								{
									writer.WriteStrPtr(SectionIDError);
									writer.WritePtr(FileAddr::NullPtr);
									writer.WriteU64(0);
									writer.WriteU64(0);
								}
							}
							writer.WriteAlignmentPadding(16);
						});
						writer.WriteU64(0);
						writer.WriteU64(0);
						writer.WriteAlignmentPadding(16);
					});
					writer.WriteU64(0);
					writer.WriteU64(0);
				}
				else if (sectionIndex == 2)
				{
					writer.WriteStrPtr(SectionIDDebug);
					writer.WriteFuncPtr([&](IO::StreamWriter& writer)
					{
						writer.WriteStr("Reserved");
						writer.WriteAlignmentPadding(16);
					});
					writer.WriteU64(0);
					writer.WriteU64(0);
				}
				else
				{
					writer.WriteStrPtr(SectionIDError);
					writer.WritePtr(FileAddr::NullPtr);
					writer.WriteU64(0);
					writer.WriteU64(0);
				}
			}
			writer.WriteAlignmentPadding(16);
		});
		writer.WriteU64(0);
		writer.WriteU64(0);
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		return IO::StreamResult::Success;
	}

	void ComfyStudioChartFile::FromChart(const Chart& sourceChart)
	{
		using namespace ChartFileFormat;

		auto addMetadata = [this](std::string_view key, std::string_view value) { if (!value.empty()) { metadata.Add(std::string(key), std::string(value)); } };
		addMetadata(SectionIDMetadataSongFileName, sourceChart.SongFileName);
		addMetadata(SectionIDMetadataSongTitle, sourceChart.Properties.Song.Title);
		addMetadata(SectionIDMetadataSongTitleReading, sourceChart.Properties.Song.TitleReading);
		addMetadata(SectionIDMetadataArtist, sourceChart.Properties.Song.Artist);
		addMetadata(SectionIDMetadataAlbum, sourceChart.Properties.Song.Album);
		addMetadata(SectionIDMetadataLyricist, sourceChart.Properties.Song.Lyricist);
		addMetadata(SectionIDMetadataArranger, sourceChart.Properties.Song.Arranger);
		addMetadata(SectionIDMetadataTrackNumber, std::to_string(sourceChart.Properties.Song.TrackNumber));
		addMetadata(SectionIDMetadataDiskNumber, std::to_string(sourceChart.Properties.Song.DiskNumber));
		addMetadata(SectionIDMetadataCreatorName, sourceChart.Properties.Creator.Name);
		addMetadata(SectionIDMetadataCreatorComment, sourceChart.Properties.Creator.Comment);
		addMetadata(SectionIDMetadataImageCoverFileName, sourceChart.Properties.Image.CoverFileName);
		addMetadata(SectionIDMetadataImageLogoFileName, sourceChart.Properties.Image.LogoFileName);
		addMetadata(SectionIDMetadataImageBackgroundFileName, sourceChart.Properties.Image.BackgroundFileName);

		for (size_t i = 0; i < sourceChart.Properties.Song.ExtraInfo.size(); i++)
		{
			char buffer[32];
			const auto& extraInfo = sourceChart.Properties.Song.ExtraInfo[i];

			if (!extraInfo.Key.empty())
				addMetadata(std::string_view(buffer, sprintf_s(buffer, SectionIDMetadataExtraInfoKeyFmt.data(), i)), extraInfo.Key);
			if (!extraInfo.Value.empty())
				addMetadata(std::string_view(buffer, sprintf_s(buffer, SectionIDMetadataExtraInfoValueFmt.data(), i)), extraInfo.Value);
		}

		chart.Scale.ButtonTypeNames.reserve(EnumCount<ButtonType>());
		static_assert(static_cast<u8>(ButtonType::Triangle) == 0);
		chart.Scale.ButtonTypeNames.emplace_back("Triangle");
		static_assert(static_cast<u8>(ButtonType::Square) == 1);
		chart.Scale.ButtonTypeNames.emplace_back("Square");
		static_assert(static_cast<u8>(ButtonType::Cross) == 2);
		chart.Scale.ButtonTypeNames.emplace_back("Cross");
		static_assert(static_cast<u8>(ButtonType::Circle) == 3);
		chart.Scale.ButtonTypeNames.emplace_back("Circle");
		static_assert(static_cast<u8>(ButtonType::SlideL) == 4);
		chart.Scale.ButtonTypeNames.emplace_back("SlideL");
		static_assert(static_cast<u8>(ButtonType::SlideR) == 5);
		chart.Scale.ButtonTypeNames.emplace_back("SlideR");
		static_assert(static_cast<u8>(EnumCount<ButtonType>()) == 6);

		chart.Scale.TicksPerBeat = BeatTick::TicksPerBeat;
		chart.Scale.PlacementAreaSize = Rules::PlacementAreaSize;
		chart.Scale.FullAngleRotation = 360.0f;

		chart.Time.SongOffset = sourceChart.StartOffset;
		chart.Time.Duration = sourceChart.Duration;
		chart.Time.SongPreviewStart = sourceChart.Properties.SongPreview.StartTime;
		chart.Time.SongPreviewDuration = sourceChart.Properties.SongPreview.Duration;

		chart.Targets = sourceChart.Targets.GetRawView();
		chart.TempoMap = sourceChart.TempoMap.GetRawView();

		switch (sourceChart.Properties.Difficulty.Type)
		{
		case Difficulty::Easy: { chart.Difficulty.Type = ChartData::DifficultyType::Easy; chart.Difficulty.Version = ChartData::DifficultyVersion::Original; break; }
		case Difficulty::Normal: { chart.Difficulty.Type = ChartData::DifficultyType::Normal; chart.Difficulty.Version = ChartData::DifficultyVersion::Original; break; }
		case Difficulty::Hard: { chart.Difficulty.Type = ChartData::DifficultyType::Hard; chart.Difficulty.Version = ChartData::DifficultyVersion::Original; break; }
		case Difficulty::Extreme: { chart.Difficulty.Type = ChartData::DifficultyType::Extreme; chart.Difficulty.Version = ChartData::DifficultyVersion::Original; break; }
		case Difficulty::ExExtreme: { chart.Difficulty.Type = ChartData::DifficultyType::Extreme; chart.Difficulty.Version = ChartData::DifficultyVersion::Extra; break; }
		}
		chart.Difficulty.LevelWhole = static_cast<ChartData::DifficultyLevelWhole>(static_cast<u8>(sourceChart.Properties.Difficulty.Level) / 2);
		chart.Difficulty.LevelFraction = (static_cast<u8>(sourceChart.Properties.Difficulty.Level) % 2 == 0) ? ChartData::DifficultyLevelFraction::Zero : ChartData::DifficultyLevelFraction::Half;

		chart.ButtonSound.ButtonID = sourceChart.Properties.ButtonSound.ButtonID;
		chart.ButtonSound.SlideID = sourceChart.Properties.ButtonSound.SlideID;
		chart.ButtonSound.ChainSlideID = sourceChart.Properties.ButtonSound.ChainSlideID;
		chart.ButtonSound.SliderTouchID = sourceChart.Properties.ButtonSound.SliderTouchID;
	}
}
