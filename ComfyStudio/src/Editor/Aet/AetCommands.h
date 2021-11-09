#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Graphics/Auth2D/Aet/AetUtil.h"

#define Define_PropertyCommand(commandName, commandString, refType, valueType, propertyName) struct COMFY_CONCAT(commandName, _Accessor) { static valueType& Get(refType& ref) { return ref.propertyName; } static void Set(refType& ref, const valueType& value) { ref.propertyName = value; }	static constexpr std::string_view Name = commandString; }; using commandName = ::Comfy::Undo::SharedPtrRefAccessorCommand<refType, valueType, COMFY_CONCAT(commandName, _Accessor)>;

#define Define_AccessorCommand(commandName, commandString, refType, valueType, getter, setter) struct COMFY_CONCAT(commandName, _Accessor) { static valueType Get(refType& ref) { return ref.getter(); } static void Set(refType& ref, const valueType& value) { ref.setter(value); }	static constexpr std::string_view Name = commandString; }; using commandName = ::Comfy::Undo::SharedPtrRefAccessorCommand<refType, valueType, COMFY_CONCAT(commandName, _Accessor)>;

namespace Comfy::Studio::Editor
{
	Define_PropertyCommand(SceneChangeName, "Scene Name Change", Graphics::Aet::Scene, std::string, Name);
	Define_PropertyCommand(SceneChangeResolution, "Scene Resolution Change", Graphics::Aet::Scene, ivec2, Resolution);
	Define_PropertyCommand(SceneChangeStartFrame, "Scene Start Frame Change", Graphics::Aet::Scene, frame_t, StartFrame);
	Define_PropertyCommand(SceneChangeEndFrame, "Scene End Frame Change", Graphics::Aet::Scene, frame_t, EndFrame);
	Define_PropertyCommand(SceneChangeFrameRate, "Scene Frame Rate Change", Graphics::Aet::Scene, frame_t, FrameRate);
	Define_PropertyCommand(SceneChangeBackgroundColor, "Scene Background Color Change", Graphics::Aet::Scene, u32, BackgroundColor);

	struct CompositionChangeNameAccessor
	{
		static std::string Get(Graphics::Aet::Composition& ref) { return std::string(ref.GetName()); }
		static void Set(Graphics::Aet::Composition& ref, const std::string& value) { ref.SetName(value); }
		static constexpr std::string_view Name = "Composition Name Change";
	};
	using CompositionChangeName = Undo::SharedPtrRefAccessorCommand<Graphics::Aet::Composition, std::string, CompositionChangeNameAccessor>;
	// Define_AccessorCommand(CompositionChangeName, "Composition Name Change", Graphics::Aet::Composition, std::string, GetName, SetName);

	Define_AccessorCommand(LayerChangeName, "Layer Name Change", Graphics::Aet::Layer, std::string, GetName, SetName);
	Define_PropertyCommand(LayerChangeStartOffset, "Layer Start Offset Change", Graphics::Aet::Layer, frame_t, StartOffset);
	Define_PropertyCommand(LayerChangeTimeScale, "Layer Time Scale Change", Graphics::Aet::Layer, float, TimeScale);
	Define_AccessorCommand(LayerChangeFlagsVisible, "Visbility Change", Graphics::Aet::Layer, bool, GetIsVisible, SetIsVisible);
	Define_AccessorCommand(LayerChangeFlagsAudible, "Audibility Change", Graphics::Aet::Layer, bool, GetIsAudible, SetIsAudible);
	Define_AccessorCommand(LayerChangeVideoItem, "Video Item Change", Graphics::Aet::Layer, std::shared_ptr<Graphics::Aet::Video>, GetVideoItem, SetItem);
	Define_AccessorCommand(LayerChangeCompItem, "Composition Item Change", Graphics::Aet::Layer, std::shared_ptr<Graphics::Aet::Composition>, GetCompItem, SetItem);
	Define_AccessorCommand(LayerChangeReferencedParentLayer, "Reference Parent Change", Graphics::Aet::Layer, std::shared_ptr<Graphics::Aet::Layer>, GetRefParentLayer, SetRefParentLayer);

	Define_PropertyCommand(AnimationDataChangeBlendMode, "Blend Mode Change", Graphics::Aet::LayerVideo, Graphics::AetBlendMode, TransferMode.BlendMode);
	Define_AccessorCommand(AnimationDataChangeUseTextureMask, "Texture Mask Change", Graphics::Aet::LayerVideo, bool, GetUseTextureMask, SetUseTextureMask);

	Define_PropertyCommand(LayerChangeMarkerName, "Marker Name Change", Graphics::Aet::Marker, std::string, Name);
	Define_PropertyCommand(LayerChangeMarkerFrame, "Marker Frame Change", Graphics::Aet::Marker, frame_t, Frame);
}

#undef Define_PropertyCommand
#undef Define_AccessorCommand

namespace Comfy::Studio::Editor
{
	class LayerChangeStartFrame : public Undo::Command
	{
	public:
		LayerChangeStartFrame(std::shared_ptr<Graphics::Aet::Layer> ref, frame_t value)
			: reference(std::move(ref)), newValue(ClampValue(value)), oldValue(reference->StartFrame)
		{
		}

	public:
		void Undo() override
		{
			reference->StartFrame = oldValue;
			OffsetKeyFrames(oldValue - newValue);
		}

		void Redo() override
		{
			reference->StartFrame = newValue;
			OffsetKeyFrames(newValue - oldValue);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get())
				return Undo::MergeResult::Failed;

			OffsetKeyFrames(oldValue - newValue);
			newValue = ClampValue(other->newValue);
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Layer Start Frame Change"; }

	private:
		void OffsetKeyFrames(frame_t increment) { if (reference->LayerVideo != nullptr) Graphics::Aet::Util::OffsetAllKeyFrames(reference->LayerVideo->Transform, increment); }
		frame_t ClampValue(frame_t value) { return Min(value, reference->EndFrame - 1.0f); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		frame_t newValue, oldValue;
	};

	class LayerChangeEndFrame : public Undo::Command
	{
	public:
		LayerChangeEndFrame(std::shared_ptr<Graphics::Aet::Layer> ref, frame_t value)
			: reference(std::move(ref)), newValue(ClampValue(value)), oldValue(reference->EndFrame)
		{
		}

	public:
		void Undo() override
		{
			reference->EndFrame = oldValue;
		}

		void Redo() override
		{
			reference->EndFrame = newValue;
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get())
				return Undo::MergeResult::Failed;

			newValue = ClampValue(other->newValue);
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Layer End Frame Change"; }

	private:
		frame_t ClampValue(frame_t value) { return Max(value, reference->StartFrame + 1.0f); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		frame_t newValue, oldValue;
	};

	class LayerAddMarker : public Undo::Command
	{
	public:
		LayerAddMarker(std::shared_ptr<Graphics::Aet::Layer> ref, std::shared_ptr<Graphics::Aet::Marker> value)
			: reference(std::move(ref)), newValue(std::move(value))
		{
		}

	public:
		void Undo() override
		{
			reference->Markers.erase(std::find(reference->Markers.begin(), reference->Markers.end(), newValue));
		}

		void Redo() override
		{
			reference->Markers.push_back(newValue);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "New Layer Marker"; }

	private:
		frame_t ClampValue(frame_t value) { return Max(value, reference->StartFrame + 1.0f); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::shared_ptr<Graphics::Aet::Marker> newValue;
	};

	class LayerDeleteMarker : public Undo::Command
	{
	public:
		LayerDeleteMarker(std::shared_ptr<Graphics::Aet::Layer> ref, int value)
			: reference(std::move(ref)), index(value)
		{
			newValue = reference->Markers.at(index);
		}

	public:
		void Undo() override
		{
			reference->Markers.insert(reference->Markers.begin() + index, newValue);
		}

		void Redo() override
		{
			reference->Markers.erase(reference->Markers.begin() + index);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Delete Layer Marker"; }

	private:
		frame_t ClampValue(frame_t value) { return Max(value, reference->StartFrame + 1.0f); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::shared_ptr<Graphics::Aet::Marker> newValue;
		int index;
	};

	class LayerMoveMarker : public Undo::Command
	{
	public:
		LayerMoveMarker(std::shared_ptr<Graphics::Aet::Layer> ref, std::tuple<int, int> value)
			: reference(std::move(ref)), newValue(value)
		{
		}

	public:
		void Undo() override
		{
			std::iter_swap(reference->Markers.begin() + std::get<1>(newValue), reference->Markers.begin() + std::get<0>(newValue));
		}

		void Redo() override
		{
			std::iter_swap(reference->Markers.begin() + std::get<0>(newValue), reference->Markers.begin() + std::get<1>(newValue));
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Move Layer Marker"; }

	private:
		frame_t ClampValue(frame_t value) { return Max(value, reference->StartFrame + 1.0f); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::tuple<int /* SourceIndex */, int /* DestinationIndex */> newValue;
	};

	class AnimationDataChangeKeyFrameValue : public Undo::Command
	{
	public:
		AnimationDataChangeKeyFrameValue(std::shared_ptr<Graphics::Aet::Layer> ref, std::tuple<Graphics::Transform2DField_Enum, frame_t, float> value)
			: reference(std::move(ref)), newValue(value)
		{
			auto existingKeyFrame = FindExistingKeyFrame();
			oldValue = (keyFrameExisted = (existingKeyFrame != nullptr)) ? existingKeyFrame->Value : GetNewValue();
		}

	public:
		void Undo() override
		{
			if (keyFrameExisted)
				FindExistingKeyFrame()->Value = oldValue;
			else
				Graphics::Aet::Util::DeleteKeyFrameAt(GetProperty().Keys, GetFrame());
		}

		void Redo() override
		{
			if (auto existingKeyFrame = FindExistingKeyFrame(); existingKeyFrame != nullptr)
				FindExistingKeyFrame()->Value = GetNewValue();
			else
				Graphics::Aet::Util::InsertKeyFrameAt(GetProperty().Keys, GetFrame(), GetNewValue());
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get() || other->GetFrame() != GetFrame() || &other->GetProperty() != &GetProperty())
				return Undo::MergeResult::Failed;

			newValue = other->newValue;
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Key Frame Change"; }

	private:
		Graphics::Aet::Property1D& GetProperty() { return reference->LayerVideo->Transform[std::get<0>(newValue)]; }
		frame_t GetFrame() const { return std::get<1>(newValue); }
		float GetNewValue() const { return std::get<2>(newValue); }
		Graphics::Aet::KeyFrame* FindExistingKeyFrame() { return Graphics::Aet::Util::GetKeyFrameAt(GetProperty(), GetFrame()); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::tuple<Graphics::Transform2DField_Enum /* Property */, frame_t /* Frame */, float /* Value */> newValue;
		float oldValue;
		bool keyFrameExisted;
	};

	class AnimationDataChangeTransform : public Undo::Command
	{
	public:
		AnimationDataChangeTransform(std::shared_ptr<Graphics::Aet::Layer> ref, std::tuple<frame_t, vec2, vec2> value) :
			reference(ref),
			newValue(value),
			keyFrameCommandPositionX(ref, std::make_tuple(Graphics::Transform2DField_PositionX, GetFrame(), GetPosition().x)),
			keyFrameCommandPositionY(ref, std::make_tuple(Graphics::Transform2DField_PositionY, GetFrame(), GetPosition().y)),
			keyFrameCommandScaleX(ref, std::make_tuple(Graphics::Transform2DField_ScaleX, GetFrame(), GetScale().x)),
			keyFrameCommandScaleY(ref, std::make_tuple(Graphics::Transform2DField_ScaleY, GetFrame(), GetScale().y))
		{
		}

	public:
		void Undo() override
		{
			keyFrameCommandPositionX.Undo();
			keyFrameCommandPositionY.Undo();
			keyFrameCommandScaleX.Undo();
			keyFrameCommandScaleY.Undo();
		}

		void Redo() override
		{
			keyFrameCommandPositionX.Redo();
			keyFrameCommandPositionY.Redo();
			keyFrameCommandScaleX.Redo();
			keyFrameCommandScaleY.Redo();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get() || other->GetFrame() != GetFrame())
				return Undo::MergeResult::Failed;

			keyFrameCommandPositionX.TryMerge(other->keyFrameCommandPositionX);
			keyFrameCommandPositionY.TryMerge(other->keyFrameCommandPositionY);
			keyFrameCommandScaleX.TryMerge(other->keyFrameCommandScaleX);
			keyFrameCommandScaleY.TryMerge(other->keyFrameCommandScaleY);

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Transform Layer"; }

	private:
		frame_t GetFrame() const { return std::get<0>(newValue); }
		vec2 GetPosition() const { return std::get<1>(newValue); }
		vec2 GetScale() const { return std::get<2>(newValue); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::tuple<frame_t /* Frame */, vec2 /* Position */, vec2 /* Scale */> newValue;
		AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY, keyFrameCommandScaleX, keyFrameCommandScaleY;
	};

	class AnimationDataChangePosition : public Undo::Command
	{
	public:
		AnimationDataChangePosition(std::shared_ptr<Graphics::Aet::Layer> ref, std::tuple<frame_t, vec2> value) :
			reference(ref),
			newValue(value),
			keyFrameCommandPositionX(ref, std::make_tuple(Graphics::Transform2DField_PositionX, GetFrame(), GetPosition().x)),
			keyFrameCommandPositionY(ref, std::make_tuple(Graphics::Transform2DField_PositionY, GetFrame(), GetPosition().y))
		{
		}

	public:
		void Undo() override
		{
			keyFrameCommandPositionX.Undo();
			keyFrameCommandPositionY.Undo();
		}

		void Redo() override
		{
			keyFrameCommandPositionX.Redo();
			keyFrameCommandPositionY.Redo();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get() || other->GetFrame() != GetFrame())
				return Undo::MergeResult::Failed;

			keyFrameCommandPositionX.TryMerge(other->keyFrameCommandPositionX);
			keyFrameCommandPositionY.TryMerge(other->keyFrameCommandPositionY);

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Move Layer"; }

	private:
		frame_t GetFrame() const { return std::get<0>(newValue); }
		vec2 GetPosition() const { return std::get<1>(newValue); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::tuple<frame_t /* Frame */, vec2 /* Position */> newValue;
		AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY;
	};

	class AnimationDataChangeScale : public Undo::Command
	{
	public:
		AnimationDataChangeScale(std::shared_ptr<Graphics::Aet::Layer> ref, std::tuple<frame_t, vec2> value) :
			reference(ref),
			newValue(value),
			keyFrameCommandScaleX(ref, std::make_tuple(Graphics::Transform2DField_ScaleX, GetFrame(), GetScale().x)),
			keyFrameCommandScaleY(ref, std::make_tuple(Graphics::Transform2DField_ScaleY, GetFrame(), GetScale().y))
		{
		}

	public:
		void Undo() override
		{
			keyFrameCommandScaleX.Undo();
			keyFrameCommandScaleY.Undo();
		}

		void Redo() override
		{
			keyFrameCommandScaleX.Redo();
			keyFrameCommandScaleY.Redo();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (other->reference.get() != reference.get() || other->GetFrame() != GetFrame())
				return Undo::MergeResult::Failed;

			keyFrameCommandScaleX.TryMerge(other->keyFrameCommandScaleX);
			keyFrameCommandScaleY.TryMerge(other->keyFrameCommandScaleY);

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Scale Layer"; }

	private:
		frame_t GetFrame() const { return std::get<0>(newValue); }
		vec2 GetScale() const { return std::get<1>(newValue); }

	private:
		std::shared_ptr<Graphics::Aet::Layer> reference;
		std::tuple<frame_t /* Frame */, vec2 /* Scale */> newValue;
		AnimationDataChangeKeyFrameValue keyFrameCommandScaleX, keyFrameCommandScaleY;
	};
}
