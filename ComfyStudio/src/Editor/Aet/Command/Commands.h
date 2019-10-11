#pragma once
#include "AetCommand.h"
#include "FileSystem/Format/AetSet.h"
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
	using namespace FileSystem;
	using namespace Graphics::Auth2D;

	constexpr float FloatInfinity = std::numeric_limits<float>::infinity();

	// ----------------------------------------------------------------------------------------------------------------------------
	enum class AetCommandType
	{
		AetChangeName,
		AetChangeResolution,
		AetChangeStartFrame,
		AetChangeFrameDuration,
		AetChangeFrameRate,
		AetChangeBackgroundColor,

		AetLayerChangeName,

		AetObjChangeName,
		AetObjChangeLoopStart,
		AetObjChangeLoopEnd,
		AetObjChangeStartOffset,
		AetObjChangePlaybackSpeed,
		AetObjChangeFlagsVisible,
		AetObjChangeFlagsAudible,
		AetObjChangeReferenceRegion,
		AetObjChangeReferenceLayer,
		AetObjChangeObjReferenceParent,

		AnimationDataChangeBlendMode,
		AnimationDataChangeUseTextureMask,

		AetObjChangeMarkerName,
		AetObjChangeMarkerFrame,

		AetObjAddMarker,
		AetObjDeleteMarker,
		AetObjMoveMarker,
		AnimationDataChangeKeyFrameValue,
		AnimationDataChangeTransform,
		AnimationDataChangePosition,
		AnimationDataChangeScale,
	};
	// ----------------------------------------------------------------------------------------------------------------------------

	// NOTE: These don't really need to be private because they are only accessed through the AetCommand interface

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_PropertyCommand(AetChangeName, "Aet Name Change", Aet, String, Name);
	Define_PropertyCommand(AetChangeResolution, "Resolution Change", Aet, ivec2, Resolution);
	Define_PropertyCommand(AetChangeStartFrame, "Aet Start Frame Change", Aet, frame_t, FrameStart);
	Define_PropertyCommand(AetChangeFrameDuration, "Aet Frame Duration Change", Aet, frame_t, FrameDuration);
	Define_PropertyCommand(AetChangeFrameRate, "Aet Frame Rate Change", Aet, frame_t, FrameRate);
	Define_PropertyCommand(AetChangeBackgroundColor, "Aet Background Color Change", Aet, uint32_t, BackgroundColor);

	Define_AccessorCommand(AetLayerChangeName, "Layer Name Change", AetLayer, String, GetName, SetName);

	Define_AccessorCommand(AetObjChangeName, "Object Name Change", AetObj, String, GetName, SetName);
	Define_PropertyCommand(AetObjChangeStartOffset, "Object Start Offset Change", AetObj, frame_t, StartOffset);
	Define_PropertyCommand(AetObjChangePlaybackSpeed, "Object Playback Speed Change", AetObj, float, PlaybackSpeed);
	Define_AccessorCommand(AetObjChangeFlagsVisible, "Visbility Change", AetObj, bool, GetIsVisible, SetIsVisible);
	Define_AccessorCommand(AetObjChangeFlagsAudible, "Audibility Change", AetObj, bool, GetIsAudible, SetIsAudible);
	Define_AccessorCommand(AetObjChangeReferenceRegion, "Region Reference Change", AetObj, RefPtr<AetRegion>, GetReferencedRegion, SetReferencedRegion);
	Define_AccessorCommand(AetObjChangeReferenceLayer, "Layer Reference Change", AetObj, RefPtr<AetLayer>, GetReferencedLayer, SetReferencedLayer);
	Define_AccessorCommand(AetObjChangeObjReferenceParent, "Reference Parent Change", AetObj, RefPtr<AetObj>, GetReferencedParentObj, SetReferencedParentObj);

	Define_PropertyCommand(AnimationDataChangeBlendMode, "Blend Mode Change", AnimationData, AetBlendMode, BlendMode);
	Define_PropertyCommand(AnimationDataChangeUseTextureMask, "Texture Mask Change", AnimationData, bool, UseTextureMask);

	Define_PropertyCommand(AetObjChangeMarkerName, "Marker Name Change", AetMarker, String, Name);
	Define_PropertyCommand(AetObjChangeMarkerFrame, "Marker Frame Change", AetMarker, frame_t, Frame);
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AetObjChangeLoopStart, "Object Loop Start Change");
private:
	RefPtr<AetObj> ref;
	frame_t newValue, oldValue;
	inline void OffsetKeyFrames(frame_t increment) { if (ref->AnimationData != nullptr) AetMgr::OffsetAllKeyFrames(ref->AnimationData->Properties, increment); }
	inline frame_t ClampValue(frame_t value) { return glm::min(value, ref->LoopEnd - 1.0f); };

public:
	AetObjChangeLoopStart(const RefPtr<AetObj>& ref, const frame_t& value) : ref(ref), newValue(ClampValue(value)) {}
	void Do() override
	{
		oldValue = ref->LoopStart;
		ref->LoopStart = newValue;
		OffsetKeyFrames(newValue - oldValue);
	}
	void Undo() override
	{
		ref->LoopStart = oldValue;
		OffsetKeyFrames(oldValue - newValue);
	}
	void Redo() override
	{
		ref->LoopStart = newValue;
		OffsetKeyFrames(newValue - oldValue);
	}
	void Update(frame_t value)
	{
		value = ClampValue(value);
		OffsetKeyFrames(value - newValue);
		newValue = value;
		ref->LoopStart = newValue;
	}
	bool CanUpdate(AetObjChangeLoopStart* newCommand)
	{
		return (&newCommand->ref->LoopStart == &ref->LoopStart);
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AetObjChangeLoopEnd, "Object Loop End Change");
private:
	RefPtr<AetObj> ref;
	float newValue, oldValue;
	inline frame_t ClampValue(frame_t value) { return glm::max(value, ref->LoopStart + 1.0f); };
public:
	AetObjChangeLoopEnd(const RefPtr<AetObj>& ref, const frame_t& value) : ref(ref), newValue(ClampValue(value)) {}
	void Do() override { oldValue = ref->LoopEnd; ref->LoopEnd = newValue; }
	void Undo() override { ref->LoopEnd = oldValue; }
	void Redo() override { ref->LoopEnd = newValue; }
	void Update(frame_t value) { newValue = ClampValue(value); ref->LoopEnd = newValue; }
	bool CanUpdate(AetObjChangeLoopEnd* newCommand) { return (&newCommand->ref->LoopEnd == &ref->LoopEnd); }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AetObjAddMarker, "New Object Marker");
private:
	RefPtr<AetObj> ref;
	RefPtr<AetMarker> newValue;
public:
	AetObjAddMarker(const RefPtr<AetObj>& ref, const RefPtr<AetMarker>& value) : ref(ref), newValue(value) {}
	void Do()	override { ref->Markers.push_back(newValue); }
	void Undo() override { ref->Markers.erase(std::find(ref->Markers.begin(), ref->Markers.end(), newValue)); }
	void Redo() override { Do(); }
	void Update(const RefPtr<AetMarker>& value) { newValue = value; Redo(); }
	bool CanUpdate(AetObjAddMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AetObjDeleteMarker, "Delete Object Marker");
private:
	RefPtr<AetObj> ref;
	RefPtr<AetMarker> newValue;
	int index;
public:
	AetObjDeleteMarker(const RefPtr<AetObj>& ref, int value) : ref(ref), index(value) {}
	void Do()	override { newValue = ref->Markers.at(index); Redo(); }
	void Undo() override { ref->Markers.insert(ref->Markers.begin() + index, newValue); }
	void Redo() override { ref->Markers.erase(ref->Markers.begin() + index); }
	void Update(const int& value) { index = value; Redo(); }
	bool CanUpdate(AetObjDeleteMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AetObjMoveMarker, "Move Object Marker");
private:
	RefPtr<AetObj> ref;
	std::tuple<int /* SourceIndex */, int /* DestinationIndex */> newValue;
public:
	AetObjMoveMarker(const RefPtr<AetObj>& ref, std::tuple<int, int> value) : ref(ref), newValue(value) {}
	void Do()	override { std::iter_swap(ref->Markers.begin() + std::get<0>(newValue), ref->Markers.begin() + std::get<1>(newValue)); }
	void Undo() override { std::iter_swap(ref->Markers.begin() + std::get<1>(newValue), ref->Markers.begin() + std::get<0>(newValue)); }
	void Redo() override { Do(); }
	void Update(const std::tuple<int, int>& value) { newValue = value; Redo(); }
	bool CanUpdate(AetObjMoveMarker* newCommand) { return false; }
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	// TODO: Add dynamic name based on the PropertyType_Enum (?)
	// NOTE: Generic change KeyFrame value command to be used by the AetInspector and other commands internally
	Define_AetCommandStart(AnimationDataChangeKeyFrameValue, "Key Frame Change");
private:
	// NOTE: Use AetObj instead of AnimationData because we need to know about the StartFrame
	RefPtr<AetObj> ref;
	std::tuple<PropertyType_Enum /* Property */, frame_t /* Frame */, float /* Value */> newValue;
	float oldValue;
	bool keyFrameExisted;

	inline KeyFrameCollection& GetKeyFrames() { return ref->AnimationData->Properties[std::get<0>(newValue)]; }
	inline frame_t GetFrame() { return std::get<1>(newValue); }
	inline float GetNewValue() { return std::get<2>(newValue); }
	inline AetKeyFrame* FindExistingKeyFrame() { return AetMgr::GetKeyFrameAt(GetKeyFrames(), GetFrame()); }

	inline void DoRedoInternal(bool writeOldValue)
	{
		AetKeyFrame* existingKeyFrame = FindExistingKeyFrame();

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

			KeyFrameCollection& keyFrames = GetKeyFrames();
			AetMgr::InsertKeyFrameAt(keyFrames, GetFrame(), GetNewValue());
		}
	}

public:
	AnimationDataChangeKeyFrameValue(const RefPtr<AetObj>& ref, std::tuple<PropertyType_Enum, frame_t, float> value) : ref(ref), newValue(value) {}

	void Do() override
	{
		DoRedoInternal(true);
	}
	void Undo() override
	{
		AetKeyFrame* existingKeyFrame = FindExistingKeyFrame();

		if (keyFrameExisted)
		{
			existingKeyFrame->Value = oldValue;
		}
		else
		{
			KeyFrameCollection& keyFrames = GetKeyFrames();
			AetMgr::DeleteKeyFrameAt(keyFrames, GetFrame());
		}
	}
	void Redo() override
	{
		DoRedoInternal(false);
	}
	void Update(const std::tuple<PropertyType_Enum, frame_t, float>& value)
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
	Define_AetCommandStart(AnimationDataChangeTransform, "Transform");
private:
	RefPtr<AetObj> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Position */, vec2 /* Scale */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY, keyFrameCommandScaleX, keyFrameCommandScaleY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetPosition() { return std::get<1>(newValue); }
	inline vec2 GetScale() { return std::get<2>(newValue); }

public:
	AnimationDataChangeTransform(const RefPtr<AetObj>& ref, std::tuple<frame_t, vec2, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandPositionX(ref, std::make_tuple(PropertyType_PositionX, GetFrame(), GetPosition().x)),
		keyFrameCommandPositionY(ref, std::make_tuple(PropertyType_PositionY, GetFrame(), GetPosition().y)),
		keyFrameCommandScaleX(ref, std::make_tuple(PropertyType_ScaleX, GetFrame(), GetScale().x)),
		keyFrameCommandScaleY(ref, std::make_tuple(PropertyType_ScaleY, GetFrame(), GetScale().y))
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
		keyFrameCommandPositionX.Update(std::make_tuple(PropertyType_PositionX, frame, GetPosition().x));
		keyFrameCommandPositionY.Update(std::make_tuple(PropertyType_PositionY, frame, GetPosition().y));
		keyFrameCommandScaleX.Update(std::make_tuple(PropertyType_ScaleX, frame, GetScale().x));
		keyFrameCommandScaleY.Update(std::make_tuple(PropertyType_ScaleY, frame, GetScale().y));
	}
	bool CanUpdate(AnimationDataChangeTransform* newCommand)
	{
		return (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AnimationDataChangePosition, "Move");
private:
	RefPtr<AetObj> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Position */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandPositionX, keyFrameCommandPositionY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetPosition() { return std::get<1>(newValue); }

public:
	AnimationDataChangePosition(const RefPtr<AetObj>& ref, std::tuple<frame_t, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandPositionX(ref, std::make_tuple(PropertyType_PositionX, GetFrame(), GetPosition().x)),
		keyFrameCommandPositionY(ref, std::make_tuple(PropertyType_PositionY, GetFrame(), GetPosition().y))
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
		keyFrameCommandPositionX.Update(std::make_tuple(PropertyType_PositionX, frame, GetPosition().x));
		keyFrameCommandPositionY.Update(std::make_tuple(PropertyType_PositionY, frame, GetPosition().y));
	}
	bool CanUpdate(AnimationDataChangePosition* newCommand)
	{
		return (newCommand->GetFrame() == GetFrame());
	}
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------------------------------------------
	Define_AetCommandStart(AnimationDataChangeScale, "Scale");
private:
	RefPtr<AetObj> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Scale */> newValue;
	AnimationDataChangeKeyFrameValue keyFrameCommandScaleX, keyFrameCommandScaleY;

	inline frame_t GetFrame() { return std::get<0>(newValue); }
	inline vec2 GetScale() { return std::get<1>(newValue); }

public:
	AnimationDataChangeScale(const RefPtr<AetObj>& ref, std::tuple<frame_t, vec2> value) : ref(ref), newValue(value),
		keyFrameCommandScaleX(ref, std::make_tuple(PropertyType_ScaleX, GetFrame(), GetScale().x)),
		keyFrameCommandScaleY(ref, std::make_tuple(PropertyType_ScaleY, GetFrame(), GetScale().y))
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
		keyFrameCommandScaleX.Update(std::make_tuple(PropertyType_ScaleX, frame, GetScale().x));
		keyFrameCommandScaleY.Update(std::make_tuple(PropertyType_ScaleY, frame, GetScale().y));
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
