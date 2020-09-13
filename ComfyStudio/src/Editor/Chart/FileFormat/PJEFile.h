#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/Chart.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Studio::Editor
{
	namespace Legacy
	{
		class PJEFile : public IO::IStreamReadable, NonCopyable
		{
		public:
			static constexpr std::string_view Extension = ".pje";

		public:
			PJEFile() = default;

		public:
			std::unique_ptr<Chart> ToChart() const;

		public:
			IO::StreamResult Read(IO::StreamReader& reader) override;

		private:
			enum class TargetType : u32
			{
				Triangle = 0x00,
				Circle = 0x01,
				Cross = 0x02,
				Square = 0x03,
				TriangleHold = 0x04,
				CircleHold = 0x05,
				CrossHold = 0x06,
				SquareHold = 0x07,
				SlideL = 0x0C,
				SlideR = 0x0D,
				SlideLChain = 0x0F,
				SlideRChain = 0x10,
				TriangleChance = 0x12,
				CircleChance = 0x13,
				CrossChance = 0x14,
				SquareChance = 0x15,
				SlideLChance = 0x17,
				SlideRChance = 0x18,
			};

			struct TimelineIndex
			{
				i32 Bar, Beat;
				operator TimelineTick() const;
			};

			struct BPMChange
			{
				TimelineIndex Index;
				f32 BPM;
			};

			struct SigChange
			{
				TimelineIndex Index;
				i32 Numerator, Denominator;
			};

			struct EditTarget
			{
				TimelineIndex Index;
				TargetType Type;
				vec2 Position;
				f32 Angle;
				f32 Frequency;
				f32 Amplitude;
				f32 Distance;
				f32 FlyTime;
			};

			u32 version;
			u32 beatsPerBar;

			i16 buttonSound;
			i16 slideSound;
			i16 chainSound;
			i16 touchSound;
			u32 pvID;
			u8 difficulty;
			u8 pvLevel;

			TimeSpan songOffset;
			TimeSpan videoOffset;
			TimelineIndex songEnd;
			std::array<char, 128> songTitle;

			std::vector<BPMChange> bpmChanges;
			std::vector<SigChange> sigChanges;
			std::vector<EditTarget> targets;

		private:
			struct ButtonTypeData
			{
				ButtonType Type;
				u8 IsHold : 1, IsChain : 1, IsChance : 1;
			};

			static ButtonTypeData ConverTargetType(TargetType type);
		};
	}
}
