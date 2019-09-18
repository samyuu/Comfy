#pragma once
#include "AetCommand.h"
#include "FileSystem/Format/AetSet.h"
#include "Graphics/Auth2D/AetMgr.h"

#define Define_AetCommandStart(commandName, commandString) class commandName : public AetCommand { friend class AetCommandManager; const char* GetName() override { return commandString; } AetCommandType GetType() override { return AetCommandType::commandName; }
#define Define_AetCommandEnd() }

#define Define_PropertyCommand(commandName, commandString, refType, valueType, propertyName)				\
		Define_AetCommandStart(commandName, commandString)													\
			RefPtr<refType> ref;																			\
			valueType newValue, oldValue;																	\
																											\
		public:																								\
			commandName(const RefPtr<refType>& ref, const valueType& value) : ref(ref), newValue(value) {}	\
			void Do()	override { oldValue = ref->propertyName; ref->propertyName = newValue; }			\
			void Undo() override { ref->propertyName = oldValue; }											\
			void Redo() override { ref->propertyName = newValue; }											\
			void Update(const valueType& value) { newValue = value; Redo(); };								\
			const void* GetDataIdentifier() override { return &ref->propertyName; };						\
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
			void Update(const valueType& value) { newValue = value; Redo(); };								\
		Define_AetCommandEnd();

namespace Editor::Command
{
	using namespace FileSystem;
	using namespace Graphics::Auth2D;

	enum class AetCommandType
	{
		AetChangeName,
		AetChangeResolution,
		AetChangeStartFrame,
		AetChangeFrameDuration,
		AetChangeFrameRate,
		AetChangeBackgroundColor,

		AetObjChangeName,
		AetObjChangeLoopStart,
		AetObjChangeLoopEnd,
		AetObjChangeStartFrame,
		AetObjChangePlaybackSpeed,
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
	};

	// NOTE: These don't really need to be private because they are only accessed through the AetCommand interface

	Define_PropertyCommand(AetChangeName, "Aet Name Change", Aet, String, Name);
	Define_PropertyCommand(AetChangeResolution, "Resolution Change", Aet, ivec2, Resolution);
	Define_PropertyCommand(AetChangeStartFrame, "Aet Start Frame Change", Aet, float, FrameStart);
	Define_PropertyCommand(AetChangeFrameDuration, "Aet Frame Duration Change", Aet, float, FrameDuration);
	Define_PropertyCommand(AetChangeFrameRate, "Aet Frame Rate Change", Aet, float, FrameRate);
	Define_PropertyCommand(AetChangeBackgroundColor, "Aet Background Color Change", Aet, uint32_t, BackgroundColor);

	Define_AccessorCommand(AetObjChangeName, "Object Name Change", AetObj, String, GetName, SetName);
	Define_PropertyCommand(AetObjChangeLoopStart, "Object Loop Start Change", AetObj, float, LoopStart);
	Define_PropertyCommand(AetObjChangeLoopEnd, "Object Loop End Change", AetObj, float, LoopEnd);
	Define_PropertyCommand(AetObjChangeStartFrame, "Object Start Frame Change", AetObj, float, StartFrame);
	Define_PropertyCommand(AetObjChangePlaybackSpeed, "Object Playback Speed Change", AetObj, float, PlaybackSpeed);
	Define_AccessorCommand(AetObjChangeReferenceRegion, "Object Region Reference Change", AetObj, RefPtr<AetRegion>, GetReferencedRegion, SetReferencedRegion);
	Define_AccessorCommand(AetObjChangeReferenceLayer, "Object Layer Reference Change", AetObj, RefPtr<AetLayer>, GetReferencedLayer, SetReferencedLayer);
	Define_AccessorCommand(AetObjChangeObjReferenceParent, "Object Reference Parent Change", AetObj, RefPtr<AetObj>, GetReferencedParentObj, SetReferencedParentObj);

	Define_PropertyCommand(AnimationDataChangeBlendMode, "Blend Mode Change", AnimationData, AetBlendMode, BlendMode);
	Define_PropertyCommand(AnimationDataChangeUseTextureMask, "Use Texture Mask Change", AnimationData, bool, UseTextureMask);

	Define_PropertyCommand(AetObjChangeMarkerName, "Object Marker Name Change", AetMarker, String, Name);
	Define_PropertyCommand(AetObjChangeMarkerFrame, "Object Marker Frame Change", AetMarker, float, Frame);


	Define_AetCommandStart(AetObjAddMarker, "New Object Marker");
private:
	RefPtr<AetObj> ref;
	RefPtr<AetMarker> newValue;
public:
	AetObjAddMarker(const RefPtr<AetObj>& ref, const RefPtr<AetMarker>& value) : ref(ref), newValue(value) {}
	void Do()	override { ref->Markers.push_back(newValue); }
	void Undo() override { ref->Markers.erase(std::find(ref->Markers.begin(), ref->Markers.end(), newValue)); }
	void Redo() override { Do(); }
	void Update(const RefPtr<AetMarker>& value) { newValue = value; Redo(); };
	Define_AetCommandEnd();


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
	void Update(const int& value) { index = value; Redo(); };
	Define_AetCommandEnd();


	Define_AetCommandStart(AetObjMoveMarker, "Move Object Marker");
private:
	RefPtr<AetObj> ref;
	std::tuple<int, int> newValue;
public:
	AetObjMoveMarker(const RefPtr<AetObj>& ref, std::tuple<int, int> value) : ref(ref), newValue(value) {}
	void Do()	override { std::iter_swap(ref->Markers.begin() + std::get<0>(newValue), ref->Markers.begin() + std::get<1>(newValue)); }
	void Undo() override { std::iter_swap(ref->Markers.begin() + std::get<1>(newValue), ref->Markers.begin() + std::get<0>(newValue)); }
	void Redo() override { Do(); }
	void Update(const std::tuple<int, int>& value) { newValue = value; Redo(); };
	Define_AetCommandEnd();


	// TODO: Consider using float KeyFrame::Frame instead of int index (?)
	Define_AetCommandStart(AnimationDataChangeKeyFrameValue, "Key Frame Value Change");
private:
	RefPtr<AnimationData> ref;
	std::tuple<PropertyType_Enum, int, float> newValue;
	float oldValue;
public:
	AnimationDataChangeKeyFrameValue(const RefPtr<AnimationData>& ref, std::tuple<PropertyType_Enum, int, float> value) : ref(ref), newValue(value) {}
	void Do()	override { oldValue = ref->Properties[std::get<0>(newValue)].at(std::get<1>(newValue)).Value; Redo(); }
	void Undo() override { ref->Properties[std::get<0>(newValue)].at(std::get<1>(newValue)).Value = oldValue; }
	void Redo() override { ref->Properties[std::get<0>(newValue)].at(std::get<1>(newValue)).Value = std::get<2>(newValue); }
	void Update(const std::tuple<PropertyType_Enum, int, float>& value) { newValue = value; Redo(); };
	const void* GetDataIdentifier() override { return &ref->Properties[std::get<0>(newValue)]; };
	Define_AetCommandEnd();
}

#undef Define_PropertyCommand
#undef Define_AccessorCommand

#undef Define_AetCommandStart
#undef Define_AetCommandEnd
