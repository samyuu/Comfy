#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "FileSystem/FileInterface.h"

namespace FileSystem
{
	// NOTE: Extra data used by Editor Components to avoid additional allocations and reduce complexity
	struct GuiTempData
	{
		// NOTE: Used for scrolling juming to a destination
		float TreeViewScrollY;
		// NOTE: Set after double clicking on a layer reference node to open it
		bool AppendOpenNode;
		// NOTE: Used to try and prevent layer name ambiguity
		int ThisIndex;
	};

	class Aet;
	class AetLayer;
	class AetSoundEffect;
	struct Sprite;

	typedef void* fileptr_t;

	enum class AetObjType : uint8_t
	{
		Nop = 0, // none
		Pic = 1, // image
		Aif = 2, // sound
		Eff = 3, // layer
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
		// Used once by "eff_mosaic01__n.pic"
		WhatTheFuck = 12,
	};

	struct AetSprite
	{
		String Name;
		uint32_t ID;
		mutable const Sprite* SpriteCache;
	};

	using SpriteCollection = Vector<AetSprite>;
	using SpriteCollectionIterator = SpriteCollection::iterator;
	using ConstSpriteCollectionIterator = SpriteCollection::const_iterator;

	class AetRegion
	{
		friend class Aet;

	public:
		AetRegion() = default;
		AetRegion(AetRegion& other) = delete;
		AetRegion& operator= (AetRegion& other) = delete;
		~AetRegion() = default;

	public:
		uint32_t Color;
		int16_t Width;
		int16_t Height;
		frame_t Frames;

		AetSprite* GetSprite(int32_t index);
		const AetSprite* GetSprite(int32_t index) const;

		AetSprite* GetFrontSprite();
		AetSprite* GetBackSprite();

		int32_t SpriteCount() const;
		SpriteCollection& GetSprites();
		const SpriteCollection& GetSprites() const;

	private:
		SpriteCollection sprites;
		fileptr_t filePosition;
	};

	struct AetMarker
	{
		AetMarker();
		AetMarker(frame_t frame, const String& name);

		frame_t Frame;
		String Name;
	};

	struct AetKeyFrame
	{
		AetKeyFrame();
		AetKeyFrame(float value);
		AetKeyFrame(frame_t frame, float value, float interpolation);
		AetKeyFrame(const AetKeyFrame& other) = default;
		AetKeyFrame& operator= (const AetKeyFrame& other) = default;
		~AetKeyFrame() = default;

		frame_t Frame;
		float Value;
		float Interpolation;
	};

	using KeyFrameCollection = Vector<AetKeyFrame>;
	using KeyFrameCollectionArray = Array<KeyFrameCollection, 8>;
	using KeyFrameCollectionArrayIterator = KeyFrameCollectionArray::iterator;
	using ConstKeyFrameCollectionArrayIterator = KeyFrameCollectionArray::const_iterator;

	struct KeyFrameProperties
	{
		static Array<const char*, 8> PropertyNames;

		KeyFrameCollectionArray KeyFrames;

		inline KeyFrameCollection& OriginX() { return KeyFrames[0]; };
		inline KeyFrameCollection& OriginY() { return KeyFrames[1]; };
		inline KeyFrameCollection& PositionX() { return KeyFrames[2]; };
		inline KeyFrameCollection& PositionY() { return KeyFrames[3]; };
		inline KeyFrameCollection& Rotation() { return KeyFrames[4]; };
		inline KeyFrameCollection& ScaleX() { return KeyFrames[5]; };
		inline KeyFrameCollection& ScaleY() { return KeyFrames[6]; };
		inline KeyFrameCollection& Opacity() { return KeyFrames[7]; };

		KeyFrameCollectionArrayIterator begin() { return KeyFrames.begin(); };
		KeyFrameCollectionArrayIterator end() { return KeyFrames.end(); };
		ConstKeyFrameCollectionArrayIterator begin() const { return KeyFrames.begin(); }
		ConstKeyFrameCollectionArrayIterator end() const { return KeyFrames.end(); }
		ConstKeyFrameCollectionArrayIterator cbegin() const { return KeyFrames.cbegin(); }
		ConstKeyFrameCollectionArrayIterator cend() const { return KeyFrames.cend(); }

		inline constexpr size_t size() const { return KeyFrames.size(); };
		inline KeyFrameCollection& at(size_t index) { return KeyFrames.at(index); };
		inline const KeyFrameCollection& at(size_t index) const { return KeyFrames.at(index); };
		inline KeyFrameCollection& operator[] (size_t index) { return KeyFrames[index]; };
	};

	struct AnimationData
	{
		static Array<const char*, 13> BlendModeNames;
		static const char* GetBlendModeName(AetBlendMode blendMode);

		AetBlendMode BlendMode;
		bool UseTextureMask;

		KeyFrameProperties Properties;
		RefPtr<KeyFrameProperties> PerspectiveProperties;
	};

	union AetObjFlags
	{
		struct
		{
			uint16_t Visible : 1;
			uint16_t Audible : 1;
		};
		uint16_t AllBits;
	};

	class AetObj
	{
		friend class Aet;
		friend class AetLayer;

	public:
		static Array<const char*, 4> TypeNames;

	public:
		mutable GuiTempData GuiData;

	public:
		AetObj();
		AetObj(AetObjType type, const String& name, AetLayer* parentLayer);
		AetObj(AetObj& other) = delete;
		AetObj& operator= (AetObj& other) = delete;
		~AetObj() = default;

		frame_t LoopStart;
		frame_t LoopEnd;
		frame_t StartFrame;
		float PlaybackSpeed;

		AetObjFlags Flags;
		unk8_t TypePaddingByte;
		AetObjType Type;

		Vector<RefPtr<AetMarker>> Markers;
		RefPtr<AnimationData> AnimationData;

		const String& GetName() const;
		void SetName(const char* value);
		void SetName(const String& value);

		bool GetIsVisible() const;
		void SetIsVisible(bool value);

		bool GetIsAudible() const;
		void SetIsAudible(bool value);

		const RefPtr<AetRegion>& GetReferencedRegion();
		const AetRegion* GetReferencedRegion() const;
		void SetReferencedRegion(const RefPtr<AetRegion>& value);

		const RefPtr<AetSoundEffect>& GetReferencedSoundEffect();
		const AetSoundEffect* GetReferencedSoundEffect() const;
		void SetReferencedSoundEffect(const RefPtr<AetSoundEffect>& value);

		const RefPtr<AetLayer>& GetReferencedLayer();
		const AetLayer* GetReferencedLayer() const;
		void SetReferencedLayer(const RefPtr<AetLayer>& value);

		const RefPtr<AetObj>& GetReferencedParentObj();
		const AetObj* GetReferencedParentObj() const;
		void SetReferencedParentObj(const RefPtr<AetObj>& value);

	public:
		Aet* GetParentAet();
		const Aet* GetParentAet() const;

		AetLayer* GetParentLayer();
		const AetLayer* GetParentLayer() const;

	private:
		String name;
		AetLayer* parentLayer;

	public:
		struct AetObjReferenceData
		{
			RefPtr<AetRegion> Region;
			RefPtr<AetSoundEffect> SoundEffect;
			RefPtr<AetLayer> Layer;
			RefPtr<AetObj> ParentObj;
		} references;

		fileptr_t filePosition;
		fileptr_t dataFilePtr;
		fileptr_t parentFilePtr;

		void Read(BinaryReader& reader);
	};

	using AetObjCollection = Vector<RefPtr<AetObj>>;
	using AetObjIterator = AetObjCollection::iterator;
	using ConstAetObjIterator = AetObjCollection::const_iterator;

	class AetLayer
	{
		friend class Aet;

	public:
		AetLayer() = default;
		AetLayer(AetLayer& other) = delete;
		AetLayer& operator= (AetLayer& other) = delete;
		~AetLayer() = default;

	public:
		mutable GuiTempData GuiData;

	public:
		inline Aet* GetParentAet() { return parentAet; };

		AetObjIterator begin() { return objects.begin(); }
		AetObjIterator end() { return objects.end(); }
		ConstAetObjIterator begin() const { return objects.begin(); }
		ConstAetObjIterator end() const { return objects.end(); }
		ConstAetObjIterator cbegin() const { return objects.cbegin(); }
		ConstAetObjIterator cend() const { return objects.cend(); }

		RefPtr<AetObj>& front() { return objects.front(); }
		RefPtr<AetObj>& back() { return objects.back(); }
		const RefPtr<AetObj>& front() const { return objects.front(); }
		const RefPtr<AetObj>& back() const { return objects.back(); }

		inline void resize(size_t newSize) { objects.resize(newSize); };
		inline void reserve(size_t newCapacity) { objects.reserve(newCapacity); };
		inline size_t size() const { return objects.size(); };

		inline RefPtr<AetObj>& at(size_t index) { return objects.at(index); };
		inline RefPtr<AetObj>& operator[] (size_t index) { return objects[index]; };

	public:
		inline AetObj* GetObjAt(int index) { return objects.at(index).get(); };
		inline const AetObj* GetObjAt(int index) const { return objects[index].get(); };

		RefPtr<AetObj> FindObj(const String& name);
		RefPtr<const AetObj> FindObj(const String& name) const;

		const Vector<String>& GetGivenNames() const;
		const String& GetCommaSeparatedNames() const;

	public:
		void AddNewObject(AetObjType type, const String& name);
		void DeleteObject(AetObj* value);

	private:
		Vector<String> givenNames;
		String commaSeparatedNames;

		Aet* parentAet;
		fileptr_t filePosition;

		AetObjCollection objects;
	};

	struct PositionOffset
	{
		KeyFrameCollection PositionX;
		KeyFrameCollection PositionY;
	};

	class AetSoundEffect
	{
		friend class Aet;

	public:
		AetSoundEffect() = default;
		AetSoundEffect(AetSoundEffect& other) = delete;
		AetSoundEffect& operator= (AetSoundEffect& other) = delete;
		~AetSoundEffect() = default;

	public:
		unk32_t Data[4];

	private:
		fileptr_t filePosition;
	};

	class Aet
	{
		friend class AetSet;
		friend class AetLayer;
		friend class AetObj;

	public:
		Aet() = default;
		Aet(Aet& other) = delete;
		Aet& operator= (Aet& other) = delete;
		~Aet() = default;

	public:
		String Name;
		frame_t FrameStart;
		frame_t FrameDuration;
		frame_t FrameRate;
		uint32_t BackgroundColor;

		ivec2 Resolution;
		RefPtr<PositionOffset> PositionOffset;

		Vector<RefPtr<AetLayer>> AetLayers;
		Vector<RefPtr<AetRegion>> AetRegions;
		Vector<RefPtr<AetSoundEffect>> AetSoundEffects;

	public:
		AetLayer* GetRootLayer();

		RefPtr<AetObj> FindObj(const String& name);
		RefPtr<const AetObj> FindObj(const String& name) const;

		int32_t FindObjIndex(AetLayer& layer, const String& name) const;

	public:
		//void AddNewLayer();
		void DeleteLayer(const RefPtr<AetLayer>& value);

	public:
		void UpdateParentPointers();

	private:
		void Read(BinaryReader& reader);
		void Write(BinaryWriter& writer);

	private:
		void InternalUpdateLayerNames();
		void InternalLinkPostRead();
		void InternalFindObjReferencedRegion(AetObj* aetObj);
		void InternalFindObjReferencedSoundEffect(AetObj* aetObj);
		void InternalFindObjReferencedLayer(AetObj* aetObj);
		void InternalFindObjReferencedParent(AetObj* aetObj);
	};

	using AetCollection = Vector<RefPtr<Aet>>;
	using AetIterator = AetCollection::iterator;
	using ConstAetIterator = AetCollection::const_iterator;

	class AetSet : public IBinaryFile
	{
	public:
		AetSet() = default;
		AetSet(AetSet& other) = delete;
		AetSet& operator= (AetSet& other) = delete;
		~AetSet() = default;

	public:
		String Name;

		AetIterator begin() { return aets.begin(); }
		AetIterator end() { return aets.end(); }
		ConstAetIterator begin() const { return aets.begin(); }
		ConstAetIterator end() const { return aets.end(); }
		ConstAetIterator cbegin() const { return aets.cbegin(); }
		ConstAetIterator cend() const { return aets.cend(); }

		RefPtr<Aet>& front() { return aets.front(); }
		RefPtr<Aet>& back() { return aets.back(); }
		const RefPtr<Aet>& front() const { return aets.front(); }
		const RefPtr<Aet>& back() const { return aets.back(); }

		inline void resize(size_t newSize) { aets.resize(newSize); };
		inline void reserve(size_t newCapacity) { aets.reserve(newCapacity); };
		inline size_t size() const { return aets.size(); };

		inline RefPtr<Aet>& at(size_t index) { return aets.at(index); };
		inline RefPtr<Aet>& operator[] (size_t index) { return aets[index]; };

		inline Aet* GetAetAt(int index) { return aets.at(index).get(); };
		inline const Aet* GetAetAt(int index) const { return aets[index].get(); };

	public:
		void ClearSpriteCache();

	public:
		virtual void Read(BinaryReader& reader) override;
		virtual void Write(BinaryWriter& writer) override;

	private:
		AetCollection aets;
	};
}