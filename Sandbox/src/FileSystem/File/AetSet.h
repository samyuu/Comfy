#pragma once
#include <vector>
#include <list>
#include <stdint.h>
#include <memory>
#include "../FileInterface.h"

namespace File
{
	struct AetLayer;
	typedef uint8_t unk8_t;
	typedef uint16_t unk16_t;
	typedef uint32_t unk32_t;
	typedef float frame_t;

	enum AetObjType : uint8_t
	{
		AetObjType_Nop = 0,
		AetObjType_Pic = 1,
		AetObjType_Aif = 2,
		AetObjType_Eff = 3,
	};

	enum AetBlendMode : uint8_t
	{
		AetBlendMode_Alpha = 3,
		AetBlendMode_Additive = 5,
		AetBlendMode_DstColorZero = 6,
		AetBlendMode_SrcAlphaOneMinusSrcColor = 7,
		AetBlendMode_Transparent = 8,
	};

	struct AetSprite
	{
		std::string Name;
		uint32_t ID;
	};

	struct SpriteEntry
	{
		void* FilePtr;
		unk32_t Unknown;
		int16_t Width;
		int16_t Height;
		frame_t Frames;
		std::vector<AetSprite> Sprites;
	};

	struct Marker
	{
		frame_t Frame;
		std::string Name;
	};

	struct KeyFrame
	{
		frame_t Frame;
		float Value;
		float Interpolation;
	};

	struct KeyFrameProperties
	{
		std::vector<KeyFrame> OriginX;
		std::vector<KeyFrame> OriginY;
		std::vector<KeyFrame> PositionX;
		std::vector<KeyFrame> PositionY;
		std::vector<KeyFrame> Rotation;
		std::vector<KeyFrame> ScaleX;
		std::vector<KeyFrame> ScaleY;
		std::vector<KeyFrame> Opacity;

		static std::array<const char*, 8> PropertyNames;
	};

	struct AnimationData
	{
		AetBlendMode BlendMode;
		unk8_t UnknownFlag0;
		bool UseTextureMask;
		unk8_t UnknownFlag2;
		std::unique_ptr<KeyFrameProperties> Properties;
		std::unique_ptr<KeyFrameProperties> PropertiesExtraData;
	};

	struct AetObj
	{
		void* FilePtr;
		std::string Name;
		frame_t LoopStart;
		frame_t LoopEnd;
		frame_t StartFrame;
		float PlaybackSpeed;
		unk8_t UnknownBytes[3];
		AetObjType Type;
		struct
		{
			void* DataFilePtr;
			void* ParentFilePtr;
			std::vector<Marker> Markers;
			AnimationData AnimationData;
			void* UnknownFilePtr;

			SpriteEntry* ReferencedSprite;
			AetLayer* ReferencedLayer;

			AetObj* ReferencedObjParent;
		};
	};

	struct AetLayer
	{
		void* FilePtr;
		std::list<AetObj> Objects;
	};

	struct AetLyo
	{
		std::string Name;
		unk32_t Unknown;
		frame_t FrameDuration;
		frame_t FrameRate;
		unk32_t MaybeColor;
		int32_t Width;
		int32_t Height;
		unk32_t DontChangeMe;
		std::list<AetLayer> AetLayers;
		std::list<SpriteEntry> SpriteEntries;
	};

	class AetSet : public IBinaryReadable
	{
	public:
		std::string Name;
		std::list<AetLyo> AetLyos;

		virtual void Read(BinaryReader& reader) override;

	private:
		static void LinkPostRead(AetSet* aetSet);
	};
}