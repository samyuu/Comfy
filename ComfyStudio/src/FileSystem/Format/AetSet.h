#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "FileSystem/FileInterface.h"

namespace FileSystem
{
	// NOTE: Extra data used by Editor Components to avoid additional allocations and reduce complexity
	struct GuiExtraData
	{
		// NOTE: For scroll jumping to a destination
		float TreeViewScrollY;
		// NOTE: Stored separately so we can expand nodes when jumping to a layer reference for example
		bool TreeViewNodeOpen;
		// NOTE: Stored to be used by the timeline
		bool TimelineNodeOpen;
		// NOTE: To try and prevent layer name ambiguity
		int ThisIndex;
	};

	class Aet;
	class AetLayer;
	class AetSoundEffect;

	// NOTE: Internal temporary file position for file parsing and writing
	typedef void* fileptr_t;

	enum class AetObjType : uint8_t
	{
		// NOTE: None
		Nop = 0,
		// NOTE: Image (-sequence) / position template
		Pic = 1,
		// NOTE: Sound effect
		Aif = 2,
		// NOTE: Layer
		Eff = 3,
	};

	enum class AetBlendMode : uint8_t
	{
		// NOTE: Normal
		Alpha = 3,
		// NOTE: Screen
		Additive = 5,
		// NOTE: Multiply
		DstColorZero = 6,
		// NOTE: Screen / Linear Dodge (Add)
		SrcAlphaOneMinusSrcColor = 7,
		// NOTE: ??
		Transparent = 8,
		// NOTE: Used once by "eff_mosaic01__n.pic"
		WhatTheFuck = 12,
	};

	struct AetSprite
	{
		// NOTE: Sprite name
		String Name;
		// NOTE: Database ID
		uint32_t ID;

		// NOTE: Editor internal cache to avoid expensive string comparisons
		mutable const struct Sprite* SpriteCache;
	};

	using SpriteCollectionIterator = Vector<AetSprite>::iterator;
	using ConstSpriteCollectionIterator = Vector<AetSprite>::const_iterator;

	// TODO: Rename to reflect sprite / position templates, sprites and image sequences
	class AetRegion
	{
		friend class Aet;

	public:
		AetRegion() = default;
		AetRegion(const AetRegion& other) = delete;
		AetRegion& operator= (const AetRegion& other) = delete;
		~AetRegion() = default;

	public:
		// NOTE: Editor internal color
		uint32_t Color;
		// NOTE: Only really used for sprite and position templates
		ivec2 Size;
		// TODO: Should be the frame count of the image sequence, should look into this further in the future
		frame_t Frames;

	public:
		AetSprite* GetSprite(int32_t index);
		const AetSprite* GetSprite(int32_t index) const;

		AetSprite* GetFrontSprite();
		AetSprite* GetBackSprite();

		int32_t SpriteCount() const;
		Vector<AetSprite>& GetSprites();
		const Vector<AetSprite>& GetSprites() const;

	private:
		Vector<AetSprite> sprites;
		fileptr_t filePosition;
	};

	struct AetMarker
	{
		AetMarker();
		AetMarker(frame_t frame, const String& name);

		// NOTE: Start frame
		frame_t Frame;
		// NOTE: Identifier
		String Name;
	};

	struct AetKeyFrame
	{
		AetKeyFrame();
		AetKeyFrame(float value);
		AetKeyFrame(frame_t frame, float value);
		AetKeyFrame(frame_t frame, float value, float curve);

		frame_t Frame;
		float Value;
		float Curve;
	};

	using KeyFrameCollection = Vector<AetKeyFrame>;
	using KeyFrameCollectionArray = Array<KeyFrameCollection, 8>;
	using KeyFrameCollectionArrayIterator = KeyFrameCollectionArray::iterator;
	using ConstKeyFrameCollectionArrayIterator = KeyFrameCollectionArray::const_iterator;

	struct KeyFrameProperties
	{
		static const Array<const char*, 8> PropertyNames;

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
		static const Array<const char*, 13> BlendModeNames;
		static const char* GetBlendModeName(AetBlendMode blendMode);

		// NOTE: Pic only sprite blend mode enum
		AetBlendMode BlendMode;
		// NOTE: Pic only texture mask bool, if true this sprite will be masked by the upper object
		bool UseTextureMask;

		// NOTE: Key frame animation data
		KeyFrameProperties Properties;
		// TODO: Perspective animation transform, not yet implemented by the editor
		RefPtr<KeyFrameProperties> PerspectiveProperties;
	};

	union AetObjFlags
	{
		// TODO: This struct is not complete, most notable there seem to be ones related to image sequences
		struct
		{
			// NOTE: Is the object visible at all? Most commonly used for masks
			uint16_t Visible : 1;
			// NOTE: Is the aif object audible? Seems to be falsely ignored ingame
			uint16_t Audible : 1;
		};
		// NOTE: Convenient field for resetting all flagss
		uint16_t AllBits;
	};

	class AetObj
	{
		friend class Aet;
		friend class AetLayer;

	public:
		static const Array<const char*, 4> TypeNames;

	public:
		AetObj();
		AetObj(AetObjType type, const String& name, AetLayer* parentLayer);
		AetObj(const AetObj& other) = delete;
		AetObj& operator= (const AetObj& other) = delete;
		~AetObj();

	public:
		mutable GuiExtraData GuiData;

		// NOTE: The first frame the object starts becoming visible.
		//		 The name 'Loop' is not entirely accurate and should perhaps be renamed
		//		 but for now it helps with differentiating it from 'StartOffset'
		frame_t LoopStart;

		// NOTE: The last frame the object is visible on
		frame_t LoopEnd;

		// NOTE: The offset the underlying referenced layer (or image sequence (?)) is offset by relative to the LoopStart. Also known as "time remapping".
		//		 Strangely some pic objects sometimes use a non-zero value
		frame_t StartOffset;

		// NOTE: The factor the underlying referenced layer (or image sequence (?)) is sped up by. Also known as "time stretching"
		float PlaybackSpeed;

		// NOTE: General flags
		AetObjFlags Flags;

		// NOTE: Unknown and doesn't seem to be used anywhere. Not always 0x00 however so might be some internal editor state such as a color enum
		unk8_t TypePaddingByte;

		// NOTE: Type of the reference data
		AetObjType Type;

		// NOTE: A list of named frame markers used for game internals
		Vector<RefPtr<AetMarker>> Markers;
		
		// NOTE: Everything render and animation related. Optional field not used by audio objects
		RefPtr<AnimationData> AnimationData;

	public:
		const String& GetName() const;
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

	using AetObjIterator = Vector<RefPtr<AetObj>>::iterator;
	using ConstAetObjIterator = Vector<RefPtr<AetObj>>::const_iterator;

	class AetLayer
	{
		friend class Aet;

	public:
		AetLayer() = default;
		AetLayer(const AetLayer& other) = delete;
		AetLayer& operator= (const AetLayer& other) = delete;
		~AetLayer() = default;

	public:
		mutable GuiExtraData GuiData;

	public:
		Aet* GetParentAet() const;
		bool IsRootLayer() const;
		
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
		const String& GetName() const;
		void SetName(const String& value);

		inline AetObj* GetObjAt(int index) { return objects.at(index).get(); };
		inline const AetObj* GetObjAt(int index) const { return objects[index].get(); };

		RefPtr<AetObj> FindObj(const String& name);
		RefPtr<const AetObj> FindObj(const String& name) const;
		
	public:
		void AddNewObject(AetObjType type, const String& name);
		void DeleteObject(AetObj* value);

	private:
		static const String rootLayerName;
		static const String unusedLayerName;

		Aet* parentAet;
		fileptr_t filePosition;

		// NOTE: The Name given to any new eff object referencing this layer. Assigned on AetSet load to the last object's name using it (= not saved if unused)
		String name;
		Vector<RefPtr<AetObj>> objects;
	};

	struct AetCamera
	{
		KeyFrameCollection PositionX;
		KeyFrameCollection PositionY;
	};

	class AetSoundEffect
	{
		friend class Aet;

	public:
		AetSoundEffect() = default;
		AetSoundEffect(const AetSoundEffect& other) = delete;
		AetSoundEffect& operator= (const AetSoundEffect& other) = delete;
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
		Aet(const Aet& other) = delete;
		Aet& operator= (const Aet& other) = delete;
		~Aet() = default;

	public:
		// NOTE: Typically "MAIN", "TOUCH" or named after the graphics mode
		String Name;

		// NOTE: Start frame of the root layer
		frame_t FrameStart;
		// NOTE: End frame of the root layer
		frame_t FrameDuration;
		// NOTE: Base framerate of the entire aet
		frame_t FrameRate;

		// NOTE: Editor internal background color
		uint32_t BackgroundColor;
		// NOTE: Editor internal base resolution
		ivec2 Resolution;

		// NOTE: Unused 2D camera for all layers
		RefPtr<AetCamera> Camera;

		// NOTE: Sub layers referenced by eff objects
		Vector<RefPtr<AetLayer>> Layers;
		// NOTE: The root layer from which all other layers will be referenced
		RefPtr<AetLayer> RootLayer;

		// NOTE: Referenced by pic objects
		Vector<RefPtr<AetRegion>> Regions;
		// NOTE: Referenced by pic objects
		Vector<RefPtr<AetSoundEffect>> SoundEffects;

	public:
		AetLayer* GetRootLayer();
		const AetLayer* GetRootLayer() const;

		RefPtr<AetObj> FindObj(const String& name);
		RefPtr<const AetObj> FindObj(const String& name) const;

		int32_t FindObjIndex(AetLayer& layer, const String& name) const;

	public:
		//void AddNewLayer();
		//void DeleteLayer(const RefPtr<AetLayer>& value);

	public:
		void UpdateParentPointers();

	private:
		void Read(BinaryReader& reader);
		void Write(BinaryWriter& writer);

	private:
		void InternalUpdateLayerNamesAfteObjectReferences();
		void InternalUpdateLayerNamesAfteObjectReferences(RefPtr<AetLayer>& aetLayer);
		void InternalLinkPostRead();
		void InternalLinkeLayerContent(RefPtr<AetLayer>& aetLayer);
		void InternalFindObjReferencedRegion(AetObj* aetObj);
		void InternalFindObjReferencedSoundEffect(AetObj* aetObj);
		void InternalFindObjReferencedLayer(AetObj* aetObj);
		void InternalFindObjReferencedParent(AetObj* aetObj);
	};

	using AetIterator = Vector<RefPtr<Aet>>::iterator;
	using ConstAetIterator = Vector<RefPtr<Aet>>::const_iterator;

	class AetSet : public IBinaryFile
	{
	public:
		AetSet() = default;
		AetSet(const AetSet& other) = delete;
		AetSet& operator= (const AetSet& other) = delete;
		~AetSet() = default;

	public:
		// TODO: File name, should probably be moved into the IReadable interface and be set OnLoad (?)
		String Name;

	public:
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
		Vector<RefPtr<Aet>> aets;
	};
}