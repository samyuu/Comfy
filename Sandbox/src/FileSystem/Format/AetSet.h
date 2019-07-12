#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"
#include <memory>
#include <vector>
#include <array>

namespace FileSystem
{
	class Aet;
	struct AetLayer;
	struct Sprite;

	typedef void* fileptr_t;

	typedef uint16_t AetTypeFlags;
	enum AetTypeFlags_Enum : AetTypeFlags
	{
		AetTypeFlags_None = 0,
		AetTypeFlags_Visible = 1 << 0,
	};

	enum class AetObjType : uint8_t
	{
		Nop = 0,
		Pic = 1,
		Aif = 2,
		Eff = 3,
	};

	enum class AetBlendMode : uint8_t
	{
		// Normal
		Alpha = 3,
		// Screen
		Additive = 5,
		// Multiply
		DstColorZero = 6,
		// Screen / Linear Dodge (Add)
		SrcAlphaOneMinusSrcColor = 7,
		// ??
		Transparent = 8,
	};

	struct AetSprite
	{
		std::string Name;
		uint32_t ID;
		Sprite* SpriteCache;
	};

	class AetRegion
	{
		friend class Aet;

	public:
		unk32_t Color;
		int16_t Width;
		int16_t Height;
		frame_t Frames;
		std::vector<AetSprite> Sprites;

	private:
		// AetSprite* dynamicSprite;

		fileptr_t filePosition;
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

	using KeyFrameCollection = std::vector<KeyFrame>;
	using KeyFrameCollectionArray = std::array<KeyFrameCollection, 8>;
	using KeyFrameCollectionArrayIterator = KeyFrameCollectionArray::iterator;
	using ConstKeyFrameCollectionArrayIterator = KeyFrameCollectionArray::const_iterator;

	struct KeyFrameProperties
	{
		static std::array<const char*, 8> PropertyNames;

		KeyFrameCollectionArray KeyFrames;

		inline KeyFrameCollection& OriginX()	{ return KeyFrames[0]; };
		inline KeyFrameCollection& OriginY()	{ return KeyFrames[1]; };
		inline KeyFrameCollection& PositionX()	{ return KeyFrames[2]; };
		inline KeyFrameCollection& PositionY()	{ return KeyFrames[3]; };
		inline KeyFrameCollection& Rotation()	{ return KeyFrames[4]; };
		inline KeyFrameCollection& ScaleX()		{ return KeyFrames[5]; };
		inline KeyFrameCollection& ScaleY()		{ return KeyFrames[6]; };
		inline KeyFrameCollection& Opacity()	{ return KeyFrames[7]; };

		KeyFrameCollectionArrayIterator begin()				{ return KeyFrames.begin(); };
		KeyFrameCollectionArrayIterator end()				{ return KeyFrames.end(); };
		ConstKeyFrameCollectionArrayIterator begin() const	{ return KeyFrames.begin(); }
		ConstKeyFrameCollectionArrayIterator end() const	{ return KeyFrames.end(); }
		ConstKeyFrameCollectionArrayIterator cbegin() const { return KeyFrames.cbegin(); }
		ConstKeyFrameCollectionArrayIterator cend() const	{ return KeyFrames.cend(); }

		inline size_t size() const								{ return KeyFrames.size(); };
		inline KeyFrameCollection& at(size_t index)				{ return KeyFrames.at(index); };
		inline KeyFrameCollection& operator[] (size_t index)	{ return KeyFrames[index]; };
	};

	struct AnimationData
	{
		AetBlendMode BlendMode;
		bool UseTextureMask;

		std::shared_ptr<KeyFrameProperties> Properties;
		std::shared_ptr<KeyFrameProperties> PerspectiveProperties;
	};

	class AetObj
	{
		friend class Aet;
		friend class AetLayer;

	public:
		static std::array<const char*, 4> TypeNames;

		frame_t LoopStart;
		frame_t LoopEnd;
		frame_t StartFrame;
		float PlaybackSpeed;

		AetTypeFlags TypeFlag;
		unk8_t UnknownTypeByte;
		AetObjType Type;

		std::vector<Marker> Markers;
		AnimationData AnimationData;

		const char* GetName();
		void SetName(const char* value);

		AetRegion* GetRegion() const;
		AetLayer* GetLayer() const;
		AetObj* GetParent() const;

	private:
		std::string name;
		Aet* parentAet;

		struct
		{
			int32_t RegionIndex;
			int32_t LayerIndex;

			int32_t ParentLayerIndex;
			int32_t ParentObjIndex;

		} references;

		fileptr_t filePosition;
		fileptr_t dataFilePtr;
		fileptr_t parentFilePtr;
		fileptr_t unknownFilePtr;

		void Read(BinaryReader& reader);
	};
	
	using AetObjCollection = std::vector<AetObj>;
	using AetObjIterator = AetObjCollection::iterator;
	using ConstAetObjIterator = AetObjCollection::const_iterator;

	class AetLayer
	{
		friend class Aet;

	public:
		std::vector<std::string> Names;
		std::string CommaSeparatedNames;

		inline int32_t GetThisIndex() { return thisIndex; };

		AetObjIterator begin()				{ return objects.begin(); }
		AetObjIterator end()				{ return objects.end(); }
		ConstAetObjIterator begin() const	{ return objects.begin(); }
		ConstAetObjIterator end() const		{ return objects.end(); }
		ConstAetObjIterator cbegin() const	{ return objects.cbegin(); }
		ConstAetObjIterator cend() const	{ return objects.cend(); }

		inline void resize(size_t newSize)					 { objects.resize(newSize); };
		inline void reserve(size_t newCapacity)				 { objects.reserve(newCapacity); };
		inline size_t size() const							 { return objects.size(); };
		inline AetObj& at(size_t index)						 { return objects.at(index); };
		inline const AetObj& at(size_t index) const			 { return objects.at(index); };
		inline AetObj& operator[] (size_t index)			 { return objects[index]; };
		inline const AetObj& operator[] (size_t index) const { return objects[index]; };

	private:
		int32_t thisIndex;
		fileptr_t filePosition;
	
		AetObjCollection objects;
	};

	class Aet
	{
		friend class AetSet;
		friend class AetObj;

	public:
		std::string Name;
		frame_t FrameDuration;
		frame_t FrameRate;
		unk32_t BackgroundColor;

		int32_t Width;
		int32_t Height;

		std::vector<AetLayer> AetLayers;
		std::vector<AetRegion> AetRegions;

		inline int32_t GetThisIndex() { return thisIndex; };

	private:
		int32_t thisIndex;
		unk32_t unknownValue;
		fileptr_t unknownFilePtr0;
		fileptr_t unknownFilePtr1;
		fileptr_t unknownFilePtr2;

		void Read(BinaryReader& reader);
		void UpdateLayerNames();
		void LinkPostRead();
		void FindObjReferencedRegion(AetObj* aetObj);
		void FindObjReferencedLayer(AetObj* aetObj);
		void FindObjReferencedParent(AetObj* aetObj);
	};

	using AetCollection = std::vector<Aet>;
	using AetIterator = AetCollection::iterator;
	using ConstAetIterator = AetCollection::const_iterator;

	class AetSet : public IBinaryReadable
	{
	public:
		std::string Name;

		AetIterator begin()				{ return aets.begin(); }
		AetIterator end()				{ return aets.end(); }
		ConstAetIterator begin() const	{ return aets.begin(); }
		ConstAetIterator end() const	{ return aets.end(); }
		ConstAetIterator cbegin() const { return aets.cbegin(); }
		ConstAetIterator cend() const	{ return aets.cend(); }

		inline void resize(size_t newSize)		{ aets.resize(newSize); };
		inline void reserve(size_t newCapacity) { aets.reserve(newCapacity); };
		inline size_t size() const				{ return aets.size(); };
		inline Aet& at(size_t index)			{ return aets.at(index); };
		inline Aet& operator[] (size_t index)	{ return aets[index]; };

		void ClearSpriteCache();
		virtual void Read(BinaryReader& reader) override;

	private:
		AetCollection aets;
	};
}