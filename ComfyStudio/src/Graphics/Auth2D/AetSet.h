#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/GraphicTypes.h"

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

namespace Graphics
{
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

	struct AetSpriteIdentifier
	{
		// NOTE: Sprite name
		std::string Name;
		// NOTE: Database ID
		uint32_t ID;

		// NOTE: Editor internal cache to avoid expensive string comparisons
		mutable const struct Spr* SpriteCache;
	};

	// TODO: Rename to reflect sprite / position templates, sprites and image sequences
	class AetRegion
	{
		friend class Aet;

	public:
		AetRegion() = default;
		AetRegion(const AetRegion&) = delete;
		AetRegion& operator= (const AetRegion&) = delete;
		~AetRegion() = default;

	public:
		// NOTE: Editor internal color
		uint32_t Color;
		// NOTE: Only really used for sprite and position templates
		ivec2 Size;
		// TODO: Should be the frame count of the image sequence, should look into this further in the future
		frame_t Frames;

	public:
		AetSpriteIdentifier* GetSprite(int32_t index);
		const AetSpriteIdentifier* GetSprite(int32_t index) const;

		AetSpriteIdentifier* GetFrontSprite();
		AetSpriteIdentifier* GetBackSprite();

		int32_t SpriteCount() const;
		std::vector<AetSpriteIdentifier>& GetSprites();
		const std::vector<AetSpriteIdentifier>& GetSprites() const;

	private:
		std::vector<AetSpriteIdentifier> sprites;
		fileptr_t filePosition;
	};

	struct AetMarker
	{
		AetMarker();
		AetMarker(frame_t frame, const std::string& name);

		// NOTE: Start frame
		frame_t Frame;
		// NOTE: Identifier
		std::string Name;
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

	using KeyFrameCollection = std::vector<AetKeyFrame>;
	using KeyFrameCollectionArray = std::array<KeyFrameCollection, 8>;
	
	struct KeyFrameProperties
	{
		static const std::array<const char*, 8> PropertyNames;

		KeyFrameCollectionArray KeyFrames;

		inline auto& OriginX() { return KeyFrames[0]; };
		inline auto& OriginY() { return KeyFrames[1]; };
		inline auto& PositionX() { return KeyFrames[2]; };
		inline auto& PositionY() { return KeyFrames[3]; };
		inline auto& Rotation() { return KeyFrames[4]; };
		inline auto& ScaleX() { return KeyFrames[5]; };
		inline auto& ScaleY() { return KeyFrames[6]; };
		inline auto& Opacity() { return KeyFrames[7]; };

		inline auto begin() { return KeyFrames.begin(); };
		inline auto end() { return KeyFrames.end(); };
		inline auto begin() const { return KeyFrames.begin(); };
		inline auto end() const { return KeyFrames.end(); };
		inline auto cbegin() const { return KeyFrames.cbegin(); };
		inline auto cend() const { return KeyFrames.cend(); };

		inline constexpr size_t size() const { return KeyFrames.size(); };
		inline auto& at(size_t index) { return KeyFrames.at(index); };
		inline auto& at(size_t index) const { return KeyFrames.at(index); };
		inline auto& operator[] (size_t index) { return KeyFrames[index]; };
	};

	struct AnimationData
	{
		static const std::array<const char*, 13> BlendModeNames;
		static const char* GetBlendModeName(Graphics::AetBlendMode blendMode);

		// NOTE: Pic only sprite blend mode enum
		Graphics::AetBlendMode BlendMode;
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
		static const std::array<const char*, 4> TypeNames;

	public:
		AetObj();
		AetObj(AetObjType type, const std::string& name, AetLayer* parentLayer);
		AetObj(const AetObj&) = delete;
		AetObj& operator= (const AetObj&) = delete;
		~AetObj();

	public:
		mutable GuiExtraData GuiData;

		// NOTE: The first frame the object starts becoming visible
		frame_t StartFrame;

		// NOTE: The last frame the object is visible on
		frame_t EndFrame;

		// NOTE: The offset the underlying referenced content is offset by relative to the StartFrame. Also known as "time remapping".
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
		std::vector<RefPtr<AetMarker>> Markers;
		
		// NOTE: Everything render and animation related. Optional field not used by audio objects
		RefPtr<AnimationData> AnimationData;

	public:
		const std::string& GetName() const;
		void SetName(const std::string& value);

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
		std::string name;
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

		void Read(FileSystem::BinaryReader& reader);
	};

	class AetLayer
	{
		friend class Aet;

	public:
		AetLayer() = default;
		AetLayer(const AetLayer&) = delete;
		AetLayer& operator= (const AetLayer&) = delete;
		~AetLayer() = default;

	public:
		mutable GuiExtraData GuiData;

	public:
		Aet* GetParentAet() const;
		bool IsRootLayer() const;
		
		inline auto begin() { return objects.begin(); };
		inline auto end() { return objects.end(); };
		inline auto begin() const { return objects.begin(); };
		inline auto end() const { return objects.end(); };
		inline auto cbegin() const { return objects.cbegin(); };
		inline auto cend() const { return objects.cend(); };

		inline RefPtr<AetObj>& front() { return objects.front(); };
		inline RefPtr<AetObj>& back() { return objects.back(); };
		inline const RefPtr<AetObj>& front() const { return objects.front(); };
		inline const RefPtr<AetObj>& back() const { return objects.back(); };

		inline void resize(size_t newSize) { objects.resize(newSize); };
		inline void reserve(size_t newCapacity) { objects.reserve(newCapacity); };
		inline size_t size() const { return objects.size(); };

		inline RefPtr<AetObj>& at(size_t index) { return objects.at(index); };
		inline RefPtr<AetObj>& operator[] (size_t index) { return objects[index]; };

	public:
		const std::string& GetName() const;
		void SetName(const std::string& value);

		inline AetObj* GetObjAt(int index) { return objects.at(index).get(); };
		inline const AetObj* GetObjAt(int index) const { return objects[index].get(); };

		RefPtr<AetObj> FindObj(const std::string& name);
		RefPtr<const AetObj> FindObj(const std::string& name) const;
		
	public:
		void AddNewObject(AetObjType type, const std::string& name);
		void DeleteObject(AetObj* value);

	private:
		static const std::string rootLayerName;
		static const std::string unusedLayerName;

		Aet* parentAet;
		fileptr_t filePosition;

		// NOTE: The Name given to any new eff object referencing this layer. Assigned on AetSet load to the last object's name using it (= not saved if unused)
		std::string name;
		std::vector<RefPtr<AetObj>> objects;
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
		AetSoundEffect(const AetSoundEffect&) = delete;
		AetSoundEffect& operator= (const AetSoundEffect&) = delete;
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
		Aet(const Aet&) = delete;
		Aet& operator= (const Aet&) = delete;
		~Aet() = default;

	public:
		// NOTE: Typically "MAIN", "TOUCH" or named after the graphics mode
		std::string Name;

		// NOTE: Start frame of the root layer
		frame_t StartFrame;
		// NOTE: End frame of the root layer
		frame_t EndFrame;
		// NOTE: Base framerate of the entire aet
		frame_t FrameRate;

		// NOTE: Editor internal background color
		uint32_t BackgroundColor;
		// NOTE: Editor internal base resolution
		ivec2 Resolution;

		// NOTE: Unused 2D camera for all layers
		RefPtr<AetCamera> Camera;

		// NOTE: Sub layers referenced by eff objects
		std::vector<RefPtr<AetLayer>> Layers;
		// NOTE: The root layer from which all other layers will be referenced
		RefPtr<AetLayer> RootLayer;

		// NOTE: Referenced by pic objects
		std::vector<RefPtr<AetRegion>> Regions;
		// NOTE: Referenced by pic objects
		std::vector<RefPtr<AetSoundEffect>> SoundEffects;

	public:
		AetLayer* GetRootLayer();
		const AetLayer* GetRootLayer() const;

		RefPtr<AetObj> FindObj(const std::string& name);
		RefPtr<const AetObj> FindObj(const std::string& name) const;

		int32_t FindObjIndex(AetLayer& layer, const std::string& name) const;

	public:
		//void AddNewLayer();
		//void DeleteLayer(const RefPtr<AetLayer>& value);

	public:
		void UpdateParentPointers();

	private:
		void Read(FileSystem::BinaryReader& reader);
		void Write(FileSystem::BinaryWriter& writer);

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

	class AetSet : public FileSystem::IBinaryFile
	{
	public:
		AetSet() = default;
		AetSet(const AetSet&) = delete;
		AetSet& operator= (const AetSet&) = delete;
		~AetSet() = default;

	public:
		// TODO: File name, should probably be moved into the IReadable interface and be set OnLoad (?)
		std::string Name;

	public:
		inline auto begin() { return aets.begin(); };
		inline auto end() { return aets.end(); };
		inline auto begin() const { return aets.begin(); };
		inline auto end() const { return aets.end(); };
		inline auto cbegin() const { return aets.cbegin(); };
		inline auto cend() const { return aets.cend(); };

		inline RefPtr<Aet>& front() { return aets.front(); };
		inline RefPtr<Aet>& back() { return aets.back(); };
		inline const RefPtr<Aet>& front() const { return aets.front(); };
		inline const RefPtr<Aet>& back() const { return aets.back(); };

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
		virtual void Read(FileSystem::BinaryReader& reader) override;
		virtual void Write(FileSystem::BinaryWriter& writer) override;

	private:
		std::vector<RefPtr<Aet>> aets;
	};
}