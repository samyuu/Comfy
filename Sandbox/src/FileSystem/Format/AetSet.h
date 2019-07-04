#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"
#include <memory>
#include <vector>
#include <list>

namespace FileSystem
{
	struct AetLayer;

	typedef uint16_t AetTypeFlags;
	enum AetTypeFlags_ : AetTypeFlags
	{
		AetTypeFlags_None = 0,
		AetTypeFlags_Visible = 1 << 0,
	};

	enum AetObjType : uint8_t
	{
		AetObjType_Nop = 0,
		AetObjType_Pic = 1,
		AetObjType_Aif = 2,
		AetObjType_Eff = 3,
	};

	enum class AetBlendMode : uint8_t
	{
		Alpha = 3,
		Additive = 5,
		DstColorZero = 6,
		SrcAlphaOneMinusSrcColor = 7,
		Transparent = 8,
	};

	struct AetSprite
	{
		std::string Name;
		uint32_t ID;
	};

	struct AetRegion
	{
		void* FilePtr;
		unk32_t Color;
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
		bool UseTextureMask;
		std::unique_ptr<KeyFrameProperties> Properties;
		std::unique_ptr<KeyFrameProperties> PerspectiveProperties;
	};

	struct AetObj
	{
		void* FilePtr;
		std::string Name;
		frame_t LoopStart;
		frame_t LoopEnd;
		frame_t StartFrame;
		float PlaybackSpeed;

		AetTypeFlags TypeFlag;
		unk8_t UnknownTypeByte;
		AetObjType Type;

		struct
		{
			void* DataFilePtr;
			void* ParentFilePtr;
			std::vector<Marker> Markers;
			AnimationData AnimationData;
			void* UnknownFilePtr;

			AetRegion* ReferencedRegion;
			AetLayer* ReferencedLayer;

			AetObj* ReferencedObjParent;
		};

		static std::array<const char*, 4> TypeNames;
	};

	struct AetLayer
	{
		std::vector<std::string> Names;
		std::string CommaSeparatedNames;
		int32_t Index;
		void* FilePtr;
		std::list<AetObj> Objects;
	};

	struct AetLyo
	{
		std::string Name;
		unk32_t BackgroundColor;
		frame_t FrameDuration;
		frame_t FrameRate;
		unk32_t MaybeColor;
		int32_t Width;
		int32_t Height;
		unk32_t DontChangeMe;
		std::list<AetLayer> AetLayers;
		std::vector<AetRegion> AetRegions;
	};

	class AetSet : public IBinaryReadable
	{
	public:
		std::string Name;
		std::list<AetLyo> AetLyos;

		void UpdateLayerNames();

		virtual void Read(BinaryReader& reader) override;

	private:
		static void LinkPostRead(AetSet* aetSet);
	};
}