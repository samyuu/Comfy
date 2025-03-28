#pragma once
#include "Types.h"
#include "../Transform2D.h"
#include "Resource/IDTypes.h"
#include "Graphics/GraphicTypes.h"
#include "IO/Stream/FileInterfaces.h"
#include <optional>
#include <variant>

namespace Comfy
{
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
		class Tex;
		struct Spr;
		class SprSet;
	}
}

namespace Comfy::Graphics::Aet
{
	class Scene;
	class Composition;
	class Audio;

	class ILayerItem
	{
	protected:
		ILayerItem() = default;
	};

	class NullItem : public ILayerItem, NonCopyable
	{
	};

	struct VideoSource
	{
		std::string Name;
		SprID ID;

		// HACK: Editor internal cache to avoid expensive string comparisons
		mutable const Spr* SprCache;
		mutable const SprSet* SprSetCache;
	};

	class Video : public ILayerItem, NonCopyable
	{
		friend class Scene;

	public:
		Video() = default;
		~Video() = default;

	public:
		u32 Color;
		ivec2 Size;
		f32 FilesPerFrame;
		std::vector<VideoSource> Sources;

	public:
		VideoSource* GetSource(int index);
		const VideoSource* GetSource(int index) const;

		VideoSource* GetFront();
		VideoSource* GetBack();

	private:
		FileAddr filePosition;
	};

	struct TransferFlags
	{
		u8 PreserveAlpha : 1;
		u8 RandomizeDissolve : 1;
	};

	enum class TrackMatte : u8
	{
		NoTrackMatte = 0,
		Alpha = 1,
		NotAlpha = 2,
		Luma = 3,
		NotLuma = 4,
	};

	struct LayerTransferMode
	{
		AetBlendMode BlendMode;
		TransferFlags Flags;
		TrackMatte TrackMatte;
	};

	struct KeyFrame
	{
		KeyFrame() = default;
		KeyFrame(float value) : Value(value) {}
		KeyFrame(frame_t frame, float value) : Frame(frame), Value(value) {}
		KeyFrame(frame_t frame, float value, float curve) : Frame(frame), Value(value), Curve(curve) {}

		frame_t Frame = 0.0f;
		float Value = 0.0f;
		float Curve = 0.0f;
	};

	struct Property1D
	{
		std::vector<KeyFrame> Keys;

		inline std::vector<KeyFrame>* operator->() { return &Keys; }
		inline const std::vector<KeyFrame>* operator->() const { return &Keys; }
	};

	struct Property2D
	{
		Property1D X, Y;
	};

	struct Property3D
	{
		Property1D X, Y, Z;
	};

	struct LayerVideo2D
	{
		static constexpr std::array<const char*, 8> FieldNames =
		{
			"Origin X",
			"Origin Y",
			"Position X",
			"Position Y",
			"Rotation",
			"Scale X",
			"Scale Y",
			"Opactiy",
		};

		Property2D Origin;
		Property2D Position;
		Property1D Rotation;
		Property2D Scale;
		Property1D Opacity;

		inline Property1D& operator[](Transform2DField field)
		{
			assert(field >= Transform2DField_OriginX && field < Transform2DField_Count);
			return (&Origin.X)[field];
		}

		inline const Property1D& operator[](Transform2DField field) const
		{
			return (*const_cast<LayerVideo2D*>(this))[field];
		}
	};

	struct LayerVideo3D
	{
		Property1D OriginZ;
		Property1D PositionZ;
		Property3D DirectionXYZ;
		Property2D RotationXY;
		Property1D ScaleZ;
	};

	struct LayerVideo
	{
		LayerTransferMode TransferMode;
		LayerVideo2D Transform;
		std::shared_ptr<LayerVideo3D> Transform3D;

		inline bool GetUseTextureMask() const { return TransferMode.TrackMatte != TrackMatte::NoTrackMatte; }
		inline void SetUseTextureMask(bool value) { TransferMode.TrackMatte = value ? TrackMatte::Alpha : TrackMatte::NoTrackMatte; }
	};

	struct LayerAudio
	{
		Property1D VolumeL;
		Property1D VolumeR;
		Property1D PanL;
		Property1D PanR;
	};

	struct LayerFlags
	{
		u16 VideoActive : 1;
		u16 AudioActive : 1;
		u16 EffectsActive : 1;
		u16 MotionBlur : 1;
		u16 FrameBlending : 1;
		u16 Locked : 1;
		u16 Shy : 1;
		u16 Collapse : 1;
		u16 AutoOrientRotation : 1;
		u16 AdjustmentLayer : 1;
		u16 TimeRemapping : 1;
		u16 LayerIs3D : 1;
		u16 LookAtCamera : 1;
		u16 LookAtPointOfInterest : 1;
		u16 Solo : 1;
		u16 MarkersLocked : 1;
		// u16 NullLayer : 1;
		// u16 HideLockedMasks : 1;
		// u16 GuideLayer : 1;
		// u16 AdvancedFrameBlending : 1;
		// u16 SubLayersRenderSeparately : 1;
		// u16 EnvironmentLayer : 1;
	};

	enum class LayerQuality : u8
	{
		None = 0,
		Wireframe = 1,
		Draft = 2,
		Best = 3,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<LayerQuality>()> LayerQualityNames =
	{
		"None",
		"Wireframe",
		"Draft",
		"Best",
	};

	enum class ItemType : u8
	{
		None = 0,
		Video = 1,
		Audio = 2,
		Composition = 3,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<ItemType>()> ItemTypeNames =
	{
		"None",
		"Video",
		"Audio",
		"Composition",
	};

	struct Marker
	{
		Marker() = default;
		Marker(frame_t frame, std::string_view name) : Frame(frame), Name(name) {}

		frame_t Frame;
		std::string Name;
	};

	class Layer : NonCopyable
	{
		friend class Scene;
		friend class Composition;

	public:
		Layer() = default;
		~Layer() = default;

	public:
		mutable GuiExtraData GuiData;

		frame_t StartFrame;
		frame_t EndFrame;
		frame_t StartOffset;
		float TimeScale;
		LayerFlags Flags;
		LayerQuality Quality;
		ItemType ItemType;
		std::vector<std::shared_ptr<Marker>> Markers;
		std::shared_ptr<LayerVideo> LayerVideo;
		std::shared_ptr<LayerAudio> LayerAudio;

		mutable struct
		{
			bool UseLayerVideo;
			bool LayerVideoNoParentTransform;
			bool UseTexSpr;
			Aet::LayerVideo LayerVideo;
			const Graphics::Tex* Tex;
			const Graphics::Spr* Spr;
		} RenderOverride;

	public:
		const std::string& GetName() const;
		void SetName(const std::string& value);

		bool GetIsVisible() const;
		void SetIsVisible(bool value);

		bool GetIsAudible() const;
		void SetIsAudible(bool value);

		const std::shared_ptr<Video>& GetVideoItem();
		const std::shared_ptr<Audio>& GetAudioItem();
		const std::shared_ptr<Composition>& GetCompItem();

		const Video* GetVideoItem() const;
		const Audio* GetAudioItem() const;
		const Composition* GetCompItem() const;

		void SetItem(const std::shared_ptr<Video>& value);
		void SetItem(const std::shared_ptr<Audio>& value);
		void SetItem(const std::shared_ptr<Composition>& value);

		const std::shared_ptr<Layer>& GetRefParentLayer();
		const Layer* GetRefParentLayer() const;
		void SetRefParentLayer(const std::shared_ptr<Layer>& value);

	public:
		std::optional<frame_t> FindMarkerFrame(std::string_view markerName) const;

	public:
		Scene* GetParentScene();
		const Scene* GetParentScene() const;

		Composition* GetParentComposition();
		const Composition* GetParentComposition() const;

	private:
		std::string name;
		Composition* parentComposition;

		struct References
		{
			std::shared_ptr<Video> Video;
			std::shared_ptr<Audio> Audio;
			std::shared_ptr<Composition> Composition;
			std::shared_ptr<Layer> ParentLayer;
		} references;

		FileAddr filePosition;
		FileAddr itemFileOffset;
		FileAddr parentFileOffset;
		FileAddr audioDataFileOffset;

		void Read(IO::StreamReader& reader);
	};

	class Composition : public ILayerItem, NonCopyable
	{
		friend class Scene;

	public:
		Composition() = default;
		~Composition() = default;

	public:
		mutable GuiExtraData GuiData;

	public:
		Scene* GetParentScene() const;
		bool IsRootComposition() const;

		inline std::vector<std::shared_ptr<Layer>>& GetLayers() { return layers; }
		inline const std::vector<std::shared_ptr<Layer>>& GetLayers() const { return layers; }

	public:
		inline std::string_view GetName() const { return givenName; }
		inline void SetName(std::string_view value) { givenName = value; }

		std::shared_ptr<Layer> FindLayer(std::string_view name);
		std::shared_ptr<const Layer> FindLayer(std::string_view name) const;

	private:
		static constexpr std::string_view rootCompositionName = "Root";
		static constexpr std::string_view unusedCompositionName = "Unused Comp";

		Scene* parentScene;
		FileAddr filePosition;

		// NOTE: The Name given to any new comp item layer referencing this comp. Assigned on AetSet load to the last layer's item name using it (= not saved if unused)
		std::string givenName;
		std::vector<std::shared_ptr<Layer>> layers;
	};

	struct Camera
	{
		Property3D Eye;
		Property3D Position;
		Property3D Direction;
		Property3D Rotation;
		Property1D Zoom;
	};

	class Audio : public ILayerItem, NonCopyable
	{
		friend class Scene;

	public:
		Audio() = default;
		~Audio() = default;

	public:
		unk32_t SoundID;

	private:
		// TODO:
		// std::string givenName;

		FileAddr filePosition;
	};

	class Scene : NonCopyable
	{
		friend class AetSet;
		friend class Composition;
		friend class Layer;

	public:
		// NOTE: Typically "MAIN", "TOUCH" or named after the screen mode
		std::string Name;

		frame_t StartFrame;
		frame_t EndFrame;
		frame_t FrameRate;

		u32 BackgroundColor;
		ivec2 Resolution;

		std::shared_ptr<Camera> Camera;

		std::vector<std::shared_ptr<Composition>> Compositions;
		std::shared_ptr<Composition> RootComposition;

		std::vector<std::shared_ptr<Video>> Videos;
		std::vector<std::shared_ptr<Audio>> Audios;

	public:
		Composition* GetRootComposition();
		const Composition* GetRootComposition() const;

		std::shared_ptr<Layer> FindLayer(std::string_view name);
		std::shared_ptr<const Layer> FindLayer(std::string_view name) const;

		int FindLayerIndex(Composition& comp, std::string_view name) const;

		template <typename Func>
		void ForEachComp(Func func)
		{
			std::for_each(Compositions.begin(), Compositions.end(), func);
			func(RootComposition);
		}

		template <typename Func>
		void ForEachComp(Func func) const
		{
			std::for_each(Compositions.begin(), Compositions.end(), func);
			func(RootComposition);
		}

	public:
		void UpdateParentPointers();

	private:
		void Read(IO::StreamReader& reader);
		void Write(IO::StreamWriter& writer);

	private:
		void UpdateCompNamesAfterLayerItems();
		void UpdateCompNamesAfterLayerItems(std::shared_ptr<Composition>& comp);
		void LinkPostRead();
		void LinkCompItems(Composition& comp);
		void FindSetLayerRefParentLayer(Layer& layer);
	};

	class AetSet final : public IO::IStreamReadable, public IO::IStreamWritable, NonCopyable
	{
	public:
		AetSet() = default;
		~AetSet() = default;

	public:
		std::string Name;

	public:
		inline std::vector<std::shared_ptr<Scene>>& GetScenes() { return scenes; }
		inline const std::vector<std::shared_ptr<Scene>>& GetScenes() const { return scenes; }

		void ClearSpriteCache();

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
		std::vector<std::shared_ptr<Scene>> scenes;
	};
}

namespace Comfy::Graphics
{
	// NOTE: Because repeating the Aet prefix in the case of client code feels a bit redundant
	using AetSet = Aet::AetSet;
}
