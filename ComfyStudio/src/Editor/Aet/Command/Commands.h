#pragma once
#include "AetCommand.h"
#include "Graphics/Auth2D/AetMgr.h"

#define Define_AetCommandStart(commandName, commandString) class commandName : public AetCommand { friend class AetCommandManager; const char* GetName() override { return commandString; } AetCommandType GetType() override { return AetCommandType::commandName; }
#define Define_AetCommandEnd() }

#define Define_PropertyCommand(commandName, commandString, refType, valueType, propertyName)							\
		Define_AetCommandStart(commandName, commandString)																\
			RefPtr<refType> ref;																						\
			valueType newValue, oldValue;																				\
																														\
		public:																											\
			commandName(const RefPtr<refType>& ref, const valueType& value) : ref(ref), newValue(value) {}				\
			void Do()	override { oldValue = ref->propertyName; ref->propertyName = newValue; }						\
			void Undo() override { ref->propertyName = oldValue; }														\
			void Redo() override { ref->propertyName = newValue; }														\
			void Update(const valueType& value) { newValue = value; Redo(); }											\
			bool CanUpdate(commandName* newCommand) { return (&newCommand->ref->propertyName == &ref->propertyName); }	\
		Define_AetCommandEnd();

#define Define_AccessorCommand(commandName, commandString, refType, valueType, getter, setter)				\
		Define_AetCommandStart(commandName, commandString)													\
			RefPtr<refType> ref;																			\
			valueType newValue, oldValue;																	\
																											\
		public:																								\
			commandName(const RefPtr<refType>& ref, const valueType& value) : ref(ref), newValue(value) {}	\
			void Do()	override { oldValue = ref->getter(); ref->setter(newValue);	}						\
			void Undo() override { ref->setter(oldValue); }													\
			void Redo() override { ref->setter(newValue); }													\
			void Update(const valueType& value) { newValue = value; Redo(); }								\
			bool CanUpdate(commandName* newCommand) { return true; }										\
		Define_AetCommandEnd();

namespace Editor::Command
{
	constexpr float FloatInfinity = std::numeric_limits<float>::infinity();

	// ----------------------------------------------------------------------------------------------------------------------------
	enum class AetCommandType
	{
		AetChangeName,
		AetChangeResolution,
		AetChangeStartFrame,
		AetChangeEndFrame,
		AetChangeFrameRate,
		AetChangeBackgroundColor,

		CompositionChangeName,

		LayerChangeName,
		LayerChangeStartFrame,
		LayerChangeEndFrame,
		LayerChangeStartOffset,
		LayerChangePlaybackSpeed,
		LayerChangeFlagsVisible,
		LayerChangeFlagsAudible,
		LayerChangeReferenceSurface,
		LayerChangeReferenceComposition,
		LayerChangeReferencedParentLayer,

		AnimationDataChangeBlendMode,
		AnimationDataChangeUseTextureMask,

		LayerChangeMarkerName,
		LayerChangeMarkerFrame,

		LayerAddMarker,
		LayerDeleteMarker,
		LayerMoveMarker,
		AnimationDataChangeKeyFrameValue,
		AnimationDataChangeTransform,
		AnimationDataChangePosition,
		AnimationDataChangeScale,
	};
	// ----------------------------------------------------------------------------------------------------------------------------

	// NOTE: These don't really need to be private because they are only accessed through the AetCommand interface

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_PropertyCommand(AetChangeName, "Aet Name Change", Graphics::Aet, std::string, Name);
	Define_PropertyCommand(AetChangeResolution, "Resolution Change", Graphics::Aet, ivec2, Resolution);
	Define_PropertyCommand(AetChangeStartFrame, "Aet Start Frame Change", Graphics::Aet, frame_t, StartFrame);
	Define_PropertyCommand(AetChangeEndFrame, "Aet End Frame Change", Graphics::Aet, frame_t, EndFrame);
	Define_PropertyCommand(AetChangeFrameRate, "Aet Frame Rate Change", Graphics::Aet, frame_t, FrameRate);
	Define_PropertyCommand(AetChangeBackgroundColor, "Aet Background Color Change", Graphics::Aet, uint32_t, BackgroundColor);

	Define_AccessorCommand(CompositionChangeName, "Composition Name Change", Graphics::AetComposition, std::string, GetName, SetName);

	Define_AccessorCommand(LayerChangeName, "Layer Name Change", Graphics::AetLayer, std::string, GetName, SetName);
	Define_PropertyCommand(LayerChangeStartOffset, "Layer Start Offset Change", Graphics::AetLayer, frame_t, StartOffset);
	Define_PropertyCommand(LayerChangePlaybackSpeed, "Layer Playback Speed Change", Graphics::AetLayer, float, PlaybackSpeed);
	Define_AccessorCommand(LayerChangeFlagsVisible, "Visbility Change", Graphics::AetLayer, bool, GetIsVisible, SetIsVisible);
	Define_AccessorCommand(LayerChangeFlagsAudible, "Audibility Change", Graphics::AetLayer, bool, GetIsAudible, SetIsAudible);
	Define_AccessorCommand(LayerChangeReferenceSurface, "Surface Reference Change", Graphics::AetLayer, RefPtr<Graphics::AetSurface>, GetReferencedSurface, SetReferencedSurface);
	Define_AccessorCommand(LayerChangeReferenceComposition, "Composition Reference Change", Graphics::AetLayer, RefPtr<Graphics::AetComposition>, GetReferencedComposition, SetReferencedComposition);
	Define_AccessorCommand(LayerChangeReferencedParentLayer, "Reference Parent Change", Graphics::AetLayer, RefPtr<Graphics::AetLayer>, GetReferencedParentLayer, SetReferencedParentLayer);

	Define_PropertyCommand(AnimationDataChangeBlendMode, "Blend Mode Change", Graphics::AetAnimationData, Graphics::AetBlendMode, BlendMode);
	Define_PropertyCommand(AnimationDataChangeUseTextureMask, "Texture Mask Change", Graphics::AetAnimationData, bool, UseTextureMask);

	Define_PropertyCommand(LayerChangeMarkerName, "Marker Name Change", Graphics::AetMarker, std::string, Name);
	Define_PropertyCommand(LayerChangeMarkerFrame, "Marker Frame Change", Graphics::AetMarker, frame_t, Frame);
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(LayerChangeStartFrame, "Layer Start Frame Change");
private:
	RefPtr<Graphics::AetLayer> ref;
	frame_t newValue, oldValue;
	inline void OffsetKeyFrames(frame_t increment) { if (ref->AnimationData != nullptr) Graphics::AetMgr::OffsetAllKeyFrames(ref->AnimationData->Properties, increment); }
	inline frame_t ClampValue(frame_t value) { return glm::min(value, ref->EndFrame - 1.0f); };

public:
	LayerChangeStartFrame(const RefPtr<Graphics::AetLayer>& ref, const frame_t& value) : ref(ref), newValue(ClampValue(value)) {}
	void Do() override
	{
		oldValue = ref->StartFrame;
		ref->StartFrame = newValue;
		OffsetKeyFrames(newValue - oldValue);
	}
	void Undo() override
	{
		ref->StartFrame = oldValue;
		OffsetKeyFrames(oldValue - newValue);
	}
	void Redo() override
	{
		ref->StartFrame = newValue;
		OffsetKeyFrames(newValue - oldValue);
	}
	void Update(frame_t value)
	{
		value = ClampValue(value);
		OffsetKeyFrames(value - newValue);
		newValue = value;
		ref->StartFrame = newValue;
	}
	bool CanUpdate(LayerChangeStartFrame* newCommand)
	{
		return (&newCommand->ref->StartFrame == &ref->StartFrame);
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(LayerChangeEndFrame, "Layer End Frame Change");
private:
	RefPtr<Graphics::AetLayer> ref;
	float newValue, oldValue;
	inline frame_t ClampValue(frame_t value) { return glm::max(value, ref->StartFrame + 1.0f); };
public:
	LayerChangeEndFrame(const RefPtr<Graphics::AetLayer>& ref, const frame_t& value) : ref(ref), newValue(ClampValue(value)) {}
	void Do() override { oldValue = ref->EndFrame; ref->EndFrame = newValue; }
	void Undo() override { ref->EndFrame = oldValue; }
	void Redo() override { ref->EndFrame = newValue; }
	void Update(frame_t value) { newValue = ClampValue(value); ref->EndFrame = newValue; }
	bool CanUpdate(LayerChangeEndFrame* newCommand) { return (&newCommand->ref->EndFrame == &ref->EndFrame); }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(LayerAddMarker, "New Layer Marker");
private:
	RefPtr<Graphics::AetLayer> ref;
	RefPtr<Graphics::AetMarker> newValue;
public:
	LayerAddMarker(const RefPtr<Graphics::AetLayer>& ref, const RefPtr<Graphics::AetMarker>& value) : ref(ref), newValue(value) {}
	void Do()	override { ref->Markers.push_back(newValue); }
	void Undo() override { ref->Markers.erase(std::find(ref->Markers.begin(), ref->Markers.end(), newValue)); }
	void Redo() override { Do(); }
	void Update(const RefPtr<Graphics::AetMarker>& value) { newValue = value; Redo(); }
	bool CanUpdate(LayerAddMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(LayerDeleteMarker, "Delete Layer Marker");
private:
	RefPtr<Graphics::AetLayer> ref;
	RefPtr<Graphics::AetMarker> newValue;
	int index;
public:
	LayerDeleteMarker(const RefPtr<Graphics::AetLayer>& ref, int value) : ref(ref), index(value) {}
	void Do()	override { newValue = ref->Markers.at(index); Redo(); }
	void Undo() override { ref->Markers.insert(ref->Markers.begin() + index, newValue); }
	void Redo() override { ref->Markers.erase(ref->Markers.begin() + index); }
	void Update(const int& value) { index = value; Redo(); }
	bool CanUpdate(LayerDeleteMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(LayerMoveMarker, "Move Layer Marker");
private:
	RefPtr<Graphics::AetLayer> ref;
	std::tuple<int /* SourceIndex */, int /* DestinationIndex */> newValue;
public:
	LayerMoveMarker(const RefPtr<Graphics::AetLayer>& ref, std::tuple<int, int> value) : ref(ref), newValue(value) {}
	void Do()	override { std::iter_swap(ref->Markers.begin() + std::get<0>(newValue), ref->Markers.begin() + std::get<1>(newValue)); }
	void Undo() override { std::iter_swap(ref->Markers.begin() + std::get<1>(newValue), ref->Markers.begin() + std::get<0>(newValue)); }
	void Redo() override { Do(); }
	void Update(const std::tuple<int, int>& value) { newValue = value; Redo(); }
	bool CanUpdate(LayerMoveMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	// TODO: Add dynamic name based on the PropertyType_Enum (?)
	// NOTE: Generic change KeyFrame value command to be used by the AetInspector and other commands internally
	Define_AetCommandStart(AnimationDataChangeKeyFrameValue, "Key Frame Change");
private:
	// NOTE: Use Layer instead of AnimationData because we need to know about the StartFrame
	RefPtr<Graphics::AetLayer> ref;
	std::tuple<Graphics::PropertyType_Enum /* Property */, frame_t /* Frame */, float /* Value */> newValue;
	float oldValue;
	bool keyFrameExisted;

	inline Graphics::KeyFrameCollection& GetKeyFrames() { return ref->AnimationData->Properties[std::get<0>(newValue)]; }
	inline frame_t GetFrame() { return std::get<1>(newValue); }
	inline float GetNewValue() { return std::get<2>(newValue); }
	inline Graphics::AetKeyFrame* FindExistingKeyFrame() { return Graphics::AetMgr::GetKeyFrameAt(GetKeyFrames(), GetFrame()); }

	inline void DoRedoInternal(bool writeOldValue)
	{
		Graphics::AetKeyFrame* existingKeyFrame = FindExistingKeyFrame();

		keyFrameExisted = (existingKeyFrame != nullptr);

		if (keyFrameExisted)
		{
			if (writeOldValue)
				oldValue = existingKeyFrame->Value;
			existingKeyFrame->Value = GetNewValue();
		}
		else
		{
			if (writeOldValue)
				oldValue = GetNewValue();

			Graphics::KeyFrameCollection& keyFrames = GetKeyFrames();
			Graphics::AetMgr::InsertKeyFrameAt(keyFrames, GetFrame(), GetNewValue());
		}
	}

public:
	AnimationDataChangeKeyFrameValue(const RefPtr<Graphics::AetLayer>& ref, std::tuple<Graphics::PropertyType_Enum, frame_t, float> value) : ref(ref), newValue(value) {}

	void Do() override
	{
		DoRedoInternal(true);
	}
	void Undo() override
	{
		Graphics::AetKeyFrame* existingKeyFrame = FindExistingKeyFrame();

		if (keyFrameExisted)
		{
			existingKeyFrame->Value = oldValue;
		}
		else
		{
			Graphics::KeyFrameCollection& keyFrames = GetKeyFrames();
			Graphics::AetMgr::DeleteKeyFrameAt(keyFrames, GetFrame());
		}
	}
	void Redo() override
	{
		DoRedoInternal(false);
	}
	void Update(const std::tuple<Graphics::PropertyType_Enum, frame_t, float>& value)
	{
		newValue = value;
		FindExistingKeyFrame()->Value = GetNewValue();
	}
	bool CanUpdate(AnimationDataChangeKeyFrameValue* newCommand)
	{
		// NOTE: The problem with checking for /* && keyFrameExisted */ is that text based input creates a new keyframe on the first letter which is far from ideal
		return (&newCommand->GetKeyFrames() == &GetKeyFrames()) && (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AnimationDataChangeTransform, "Transform Layer");
private:
	RefPtr<Graphics::AetLayer> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Position */, vec2 /* Scale */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY, keyFrameCommandScaleX, keyFrameCommandScaleY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetPosition() { return std::get<1>(newValue); }
	inline vec2 GetScale() { return std::get<2>(newValue); }

public:
	AnimationDataChangeTransform(const RefPtr<Graphics::AetLayer>& ref, std::tuple<frame_t, vec2, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandPositionX(ref, std::make_tuple(Graphics::PropertyType_PositionX, GetFrame(), GetPosition().x)),
		keyFrameCommandPositionY(ref, std::make_tuple(Graphics::PropertyType_PositionY, GetFrame(), GetPosition().y)),
		keyFrameCommandScaleX(ref, std::make_tuple(Graphics::PropertyType_ScaleX, GetFrame(), GetScale().x)),
		keyFrameCommandScaleY(ref, std::make_tuple(Graphics::PropertyType_ScaleY, GetFrame(), GetScale().y))
	{
	}
	void Do() override
	{
		keyFrameCommandPositionX.Do(); keyFrameCommandPositionY.Do();
		keyFrameCommandScaleX.Do(); keyFrameCommandScaleY.Do();
	}
	void Undo() override
	{
		keyFrameCommandPositionX.Undo(); keyFrameCommandPositionY.Undo();
		keyFrameCommandScaleX.Undo(); keyFrameCommandScaleY.Undo();
	}
	void Redo() override
	{
		keyFrameCommandPositionX.Redo(); keyFrameCommandPositionY.Redo();
		keyFrameCommandScaleX.Redo(); keyFrameCommandScaleY.Redo();
	}
	void Update(const std::tuple<float, vec2, vec2>& value)
	{
		newValue = value;
		float frame = GetFrame();
		keyFrameCommandPositionX.Update(std::make_tuple(Graphics::PropertyType_PositionX, frame, GetPosition().x));
		keyFrameCommandPositionY.Update(std::make_tuple(Graphics::PropertyType_PositionY, frame, GetPosition().y));
		keyFrameCommandScaleX.Update(std::make_tuple(Graphics::PropertyType_ScaleX, frame, GetScale().x));
		keyFrameCommandScaleY.Update(std::make_tuple(Graphics::PropertyType_ScaleY, frame, GetScale().y));
	}
	bool CanUpdate(AnimationDataChangeTransform* newCommand)
	{
		return (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AnimationDataChangePosition, "Move Layer");
private:
	RefPtr<Graphics::AetLayer> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Position */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetPosition() { return std::get<1>(newValue); }

public:
	AnimationDataChangePosition(const RefPtr<Graphics::AetLayer>& ref, std::tuple<frame_t, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandPositionX(ref, std::make_tuple(Graphics::PropertyType_PositionX, GetFrame(), GetPosition().x)),
		keyFrameCommandPositionY(ref, std::make_tuple(Graphics::PropertyType_PositionY, GetFrame(), GetPosition().y))
	{
	}
	void Do() override
	{
		keyFrameCommandPositionX.Do(); keyFrameCommandPositionY.Do();
	}
	void Undo() override
	{
		keyFrameCommandPositionX.Undo(); keyFrameCommandPositionY.Undo();
	}
	void Redo() override
	{
		keyFrameCommandPositionX.Redo(); keyFrameCommandPositionY.Redo();
	}
	void Update(const std::tuple<float, vec2>& value)
	{
		newValue = value;
		float frame = GetFrame();
		keyFrameCommandPositionX.Update(std::make_tuple(Graphics::PropertyType_PositionX, frame, GetPosition().x));
		keyFrameCommandPositionY.Update(std::make_tuple(Graphics::PropertyType_PositionY, frame, GetPosition().y));
	}
	bool CanUpdate(AnimationDataChangePosition* newCommand)
	{
		return (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AnimationDataChangeScale, "Scale Layer");
private:
	RefPtr<Graphics::AetLayer> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Scale */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandScaleX, keyFrameCommandScaleY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetScale() { return std::get<1>(newValue); }

public:
	AnimationDataChangeScale(const RefPtr<Graphics::AetLayer>& ref, std::tuple<frame_t, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandScaleX(ref, std::make_tuple(Graphics::PropertyType_ScaleX, GetFrame(), GetScale().x)),
		keyFrameCommandScaleY(ref, std::make_tuple(Graphics::PropertyType_ScaleY, GetFrame(), GetScale().y))
	{
	}
	void Do() override
	{
		keyFrameCommandScaleX.Do(); keyFrameCommandScaleY.Do();
	}
	void Undo() override
	{
		keyFrameCommandScaleX.Undo(); keyFrameCommandScaleY.Undo();
	}
	void Redo() override
	{
		keyFrameCommandScaleX.Redo(); keyFrameCommandScaleY.Redo();
	}
	void Update(const std::tuple<float, vec2>& value)
	{
		newValue = value;
		float frame = GetFrame();
		keyFrameCommandScaleX.Update(std::make_tuple(Graphics::PropertyType_ScaleX, frame, GetScale().x));
		keyFrameCommandScaleY.Update(std::make_tuple(Graphics::PropertyType_ScaleY, frame, GetScale().y));
	}
	bool CanUpdate(AnimationDataChangeScale* newCommand)
	{
		return (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------
}

#undef Define_PropertyCommand
#undef Define_AccessorCommand

#undef Define_AetCommandStart
#undef Define_AetCommandEnd
