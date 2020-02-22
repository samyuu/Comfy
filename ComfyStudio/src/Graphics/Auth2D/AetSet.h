#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/IDTypes.h"
#include "Transform2D.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/GraphicsTypes.h"

// NOTE: Extra data used by Editor Components to avoid additional allocations and reduce complexity
struct GuiExtraData
{
	// NOTE: For scroll jumping to a destination
	float TreeViewScrollY;
	// NOTE: Stored separately so we can expand nodes when jumping to a composition reference for example
	bool TreeViewNodeOpen;
	// NOTE: Stored to be used by the timeline
	bool TimelineNodeOpen;
	// NOTE: To try and prevent composition name ambiguity
	int ThisIndex;
};

namespace Graphics
{
	// NOTE: Aet related types are prefixed with "Aet" but object instances of them should never use these prefixes as they are redundant.
	//		 "Composition" should be abbreviated to "Comp" in parameters, locals temporaries but not in function names or important member fields

	class Aet;
	class AetComposition;
	class AetSoundEffect;

	// NOTE: Internal temporary file position for file parsing and writing
	typedef void* fileptr_t;

	enum class AetLayerType : uint8_t
	{
		// NOTE: None
		Nop = 0,
		// NOTE: Image (-sequence) / position template / placeholder
		Pic = 1,
		// NOTE: Sound effect
		Aif = 2,
		// NOTE: Composition
		Eff = 3,
	};

	struct AetSpriteIdentifier
	{
		// NOTE: Sprite name
		std::string Name;
		// NOTE: Database ID
		SprID ID;

		// NOTE: Editor internal cache to avoid expensive string comparisons
		mutable const struct Spr* SpriteCache;
	};

	// TODO: Rename to reflect sprite / position templates, sprites and image sequences
	class AetSurface : NonCopyable
	{
		friend class Aet;

	public:
		AetSurface() = default;
		~AetSurface() = default;

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

	struct AetProperty1D
	{
		std::vector<AetKeyFrame> Keys;

		inline std::vector<AetKeyFrame>* operator->() { return &Keys; }
		inline const std::vector<AetKeyFrame>* operator->() const { return &Keys; }
	};

	struct AetProperty2D
	{
		AetProperty1D X, Y;
	};

	struct AetTransform
	{
		static const std::array<const char*, 8> FieldNames;

		AetProperty2D Origin;
		AetProperty2D Position;
		AetProperty1D Rotation;
		AetProperty2D Scale;
		AetProperty1D Opacity;

		inline AetProperty1D& operator[](Transform2DField field)
		{
			assert(field >= Transform2DField_OriginX && field < Transform2DField_Count);
			return (&Origin.X)[field];
		}

		inline const AetProperty1D& operator[](Transform2DField field) const
		{
			return (*const_cast<AetTransform*>(this))[field];
		}
	};

	struct AetAnimationData
	{
		// NOTE: Pic only sprite blend mode enum
		AetBlendMode BlendMode;
		// NOTE: Pic only texture mask bool, if true this sprite will be masked by the upper layer
		bool UseTextureMask;
		// NOTE: Key frame animation data
		AetTransform Transform;
		// TODO: Perspective animation transform, not yet implemented by the editor
		RefPtr<AetTransform> PerspectiveTransform;
	};

	union AetLayerFlags
	{
		// TODO: This struct is not complete, most notable there seem to be ones related to image sequences
		struct
		{
			// NOTE: Is the layer visible at all? Most commonly used for masks
			uint16_t Visible : 1;
			// NOTE: Is the aif layer audible? Seems to be falsely ignored ingame
			uint16_t Audible : 1;
		};
		// NOTE: Convenient field for resetting all flagss
		uint16_t AllBits;
	};

	class AetLayer : NonCopyable
	{
		friend class Aet;
		friend class AetComposition;

	public:
		static const std::array<const char*, 4> TypeNames;

	public:
		AetLayer();
		AetLayer(AetLayerType type, const std::string& name, AetComposition* parentComp);
		~AetLayer();

	public:
		mutable GuiExtraData GuiData;

		// NOTE: The first frame the layer starts becoming visible
		frame_t StartFrame;

		// NOTE: The last frame the layer is visible on
		frame_t EndFrame;

		// NOTE: The offset the underlying referenced content is offset by relative to the StartFrame. Also known as "time remapping".
		//		 Strangely some pic layers sometimes use a non-zero value
		frame_t StartOffset;

		// NOTE: The factor the underlying referenced composition or image sequence is sped up by. Also known as "time stretching"
		float PlaybackSpeed;

		// NOTE: General flags
		AetLayerFlags Flags;

		// NOTE: Unknown and doesn't seem to be used anywhere. Not always 0x00 however so might be some internal editor state such as a color enum
		unk8_t TypePaddingByte;

		// NOTE: Type of the reference data
		AetLayerType Type;

		// NOTE: A list of named frame markers used for game internals
		std::vector<RefPtr<AetMarker>> Markers;

		// NOTE: Everything render and animation related. Optional field not used by audio layers
		RefPtr<AetAnimationData> AnimationData;

	public:
		const std::string& GetName() const;
		void SetName(const std::string& value);

		bool GetIsVisible() const;
		void SetIsVisible(bool value);

		bool GetIsAudible() const;
		void SetIsAudible(bool value);

		const RefPtr<AetSurface>& GetReferencedSurface();
		const AetSurface* GetReferencedSurface() const;
		void SetReferencedSurface(const RefPtr<AetSurface>& value);

		const RefPtr<AetSoundEffect>& GetReferencedSoundEffect();
		const AetSoundEffect* GetReferencedSoundEffect() const;
		void SetReferencedSoundEffect(const RefPtr<AetSoundEffect>& value);

		const RefPtr<AetComposition>& GetReferencedComposition();
		const AetComposition* GetReferencedComposition() const;
		void SetReferencedComposition(const RefPtr<AetComposition>& value);

		const RefPtr<AetLayer>& GetReferencedParentLayer();
		const AetLayer* GetReferencedParentLayer() const;
		void SetReferencedParentLayer(const RefPtr<AetLayer>& value);

	public:
		Aet* GetParentAet();
		const Aet* GetParentAet() const;

		AetComposition* GetParentComposition();
		const AetComposition* GetParentComposition() const;

	private:
		std::string name;
		AetComposition* parentComposition;

		struct AetLayerReferenceData
		{
			RefPtr<AetSurface> Surface;
			RefPtr<AetSoundEffect> SoundEffect;
			RefPtr<AetComposition> Composition;
			RefPtr<AetLayer> ParentLayer;
		} references;

		fileptr_t filePosition;
		fileptr_t dataFilePtr;
		fileptr_t parentFilePtr;
		fileptr_t audioDataFilePtr;

		void Read(FileSystem::BinaryReader& reader);
	};

	class AetComposition : NonCopyable
	{
		friend class Aet;

	public:
		AetComposition() = default;
		~AetComposition() = default;

	public:
		mutable GuiExtraData GuiData;

	public:
		Aet* GetParentAet() const;
		bool IsRootComposition() const;

		inline auto begin() { return layers.begin(); };
		inline auto end() { return layers.end(); };
		inline auto begin() const { return layers.begin(); };
		inline auto end() const { return layers.end(); };
		inline auto cbegin() const { return layers.cbegin(); };
		inline auto cend() const { return layers.cend(); };

		inline RefPtr<AetLayer>& front() { return layers.front(); };
		inline RefPtr<AetLayer>& back() { return layers.back(); };
		inline const RefPtr<AetLayer>& front() const { return layers.front(); };
		inline const RefPtr<AetLayer>& back() const { return layers.back(); };

		inline void resize(size_t newSize) { layers.resize(newSize); };
		inline void reserve(size_t newCapacity) { layers.reserve(newCapacity); };
		inline size_t size() const { return layers.size(); };

		inline RefPtr<AetLayer>& at(size_t index) { return layers.at(index); };
		inline RefPtr<AetLayer>& operator[](size_t index) { return layers[index]; };

	public:
		const std::string& GetName() const;
		void SetName(const std::string& value);

		inline AetLayer* GetLayerAt(int index) { return layers.at(index).get(); };
		inline const AetLayer* GetLayerAt(int index) const { return layers[index].get(); };

		RefPtr<AetLayer> FindLayer(const std::string& name);
		RefPtr<const AetLayer> FindLayer(const std::string& name) const;

	public:
		void AddNewLayer(AetLayerType type, const std::string& name);
		void DeleteLayer(AetLayer* value);

	private:
		static const std::string rootCompositionName;
		static const std::string unusedCompositionName;

		Aet* parentAet;
		fileptr_t filePosition;

		// NOTE: The Name given to any new eff layer referencing this composition. Assigned on AetSet load to the last layer's name using it (= not saved if unused)
		std::string givenName;
		std::vector<RefPtr<AetLayer>> layers;
	};

	struct AetCamera
	{
		AetProperty2D Position;
	};

	class AetSoundEffect : NonCopyable
	{
		friend class Aet;

	public:
		AetSoundEffect() = default;
		~AetSoundEffect() = default;

	public:
		unk32_t Data;

	private:
		// TODO:
		// std::string givenName;

		fileptr_t filePosition;
	};

	class Aet : NonCopyable
	{
		friend class AetSet;
		friend class AetComposition;
		friend class AetLayer;

	public:
		Aet() = default;
		~Aet() = default;

	public:
		// NOTE: Typically "MAIN", "TOUCH" or named after the display mode
		std::string Name;

		// NOTE: Start frame of the root composition
		frame_t StartFrame;
		// NOTE: End frame of the root composition
		frame_t EndFrame;
		// NOTE: Base framerate of the entire aet
		frame_t FrameRate;

		// NOTE: Editor internal background color
		uint32_t BackgroundColor;
		// NOTE: Editor internal base resolution
		ivec2 Resolution;

		// NOTE: Unused 2D camera for all compositions
		RefPtr<AetCamera> Camera;

		// NOTE: Sub compositions referenced by eff layers
		std::vector<RefPtr<AetComposition>> Compositions;
		// NOTE: The root composition from which all other compositions will be referenced
		RefPtr<AetComposition> RootComposition;

		// NOTE: Referenced by pic layers
		std::vector<RefPtr<AetSurface>> Surfaces;
		// NOTE: Referenced by pic layers
		std::vector<RefPtr<AetSoundEffect>> SoundEffects;

	public:
		AetComposition* GetRootComposition();
		const AetComposition* GetRootComposition() const;

		RefPtr<AetLayer> FindLayer(const std::string& name);
		RefPtr<const AetLayer> FindLayer(const std::string& name) const;

		int32_t FindLayerIndex(AetComposition& comp, const std::string& name) const;

	public:
		//void AddNewComposition();
		//void DeleteComposition(const RefPtr<AetComposition>& value);

	public:
		void UpdateParentPointers();

	private:
		void Read(FileSystem::BinaryReader& reader);
		void Write(FileSystem::BinaryWriter& writer);

	private:
		void InternalUpdateCompositionNamesAfterLayerReferences();
		void InternalUpdateCompositionNamesAfterLayerReferences(RefPtr<AetComposition>& comp);
		void InternalLinkPostRead();
		void InternalLinkeCompositionContent(RefPtr<AetComposition>& comp);
		void InternalFindLayerReferencedSurface(AetLayer* layer);
		void InternalFindLayerReferencedSoundEffect(AetLayer* layer);
		void InternalFindLayerReferencedComposition(AetLayer* layer);
		void InternalFindLayerReferencedParent(AetLayer* layer);
	};

	class AetSet final : public FileSystem::IBinaryReadWritable, NonCopyable
	{
	public:
		AetSet() = default;
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
		inline RefPtr<Aet>& operator[](size_t index) { return aets[index]; };

		inline Aet* GetAetAt(int index) { return aets.at(index).get(); };
		inline const Aet* GetAetAt(int index) const { return aets[index].get(); };

	public:
		void ClearSpriteCache();

	public:
		void Read(FileSystem::BinaryReader& reader) override;
		void Write(FileSystem::BinaryWriter& writer) override;

	private:
		std::vector<RefPtr<Aet>> aets;
	};
}