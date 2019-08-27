#pragma once
#include "AetCommand.h"
#include "FileSystem/Format/AetSet.h"

#define DeclarePropertyCommandClass(commandName, commandString, refType, valueType, propertyName)					\
		class commandName : public AetCommand																		\
		{																											\
			RefPtr<refType> ref;																					\
			valueType newValue, oldValue;																			\
																													\
		public:																										\
			commandName(const RefPtr<refType>& ref, const valueType& value) : ref(ref), newValue(value) {}			\
																													\
			void Do()	override			{ oldValue = ref->propertyName; ref->propertyName = newValue; }			\
			void Undo() override			{ ref->propertyName = oldValue; }										\
			void Redo() override			{ ref->propertyName = newValue; }										\
			const char* GetName() override	{ return commandString;			}										\
		};																											\

#define DeclareAccessorCommandClass(commandName, commandString, refType, valueType, getter, setter)					\
		class commandName : public AetCommand																		\
		{																											\
			RefPtr<refType> ref;																					\
			valueType newValue, oldValue;																			\
																													\
		public:																										\
			commandName(const RefPtr<refType>& ref, const valueType& value) : ref(ref), newValue(value) {}			\
																													\
			void Do()	override			{ oldValue = ref->getter(); ref->setter(newValue); }					\
			void Undo() override			{ ref->setter(oldValue); }												\
			void Redo() override			{ ref->setter(newValue); }												\
			const char* GetName() override	{ return commandString;	 }												\
		};																											\

namespace Editor::Command
{
	using namespace FileSystem;

	DeclarePropertyCommandClass(AetChangeName, "Change Aet Name", Aet, std::string, Name);
	DeclarePropertyCommandClass(AetChangeResolution, "Change Resolution", Aet, ivec2, Resolution);
	DeclarePropertyCommandClass(AetChangeStartFrame, "Change Aet Start Frame", Aet, float, FrameStart);
	DeclarePropertyCommandClass(AetChangeFrameDuration, "Change Aet Frame Duration", Aet, float, FrameDuration);
	DeclarePropertyCommandClass(AetChangeFrameRate, "Change Aet Frame Rate", Aet, float, FrameRate);
	DeclarePropertyCommandClass(AetChangeBackgroundColor, "Change Aet Background Color", Aet, uint32_t, BackgroundColor);

	DeclareAccessorCommandClass(AetObjChangeName, "Change Object Name", AetObj, std::string, GetName, SetName);
	DeclarePropertyCommandClass(AetObjChangeLoopStart, "Change Object Loop Start", AetObj, float, LoopStart);
	DeclarePropertyCommandClass(AetObjChangeLoopEnd, "Change Object Loop End", AetObj, float, LoopEnd);
	DeclarePropertyCommandClass(AetObjChangeStartFrame, "Change Object Start Frame", AetObj, float, StartFrame);
	DeclarePropertyCommandClass(AetObjChangePlaybackSpeed, "Change Object Playback Speed", AetObj, float, PlaybackSpeed);
	DeclareAccessorCommandClass(AetObjChangeReferenceRegion, "Change Object Region Reference", AetObj, RefPtr<AetRegion>, GetReferencedRegion, SetReferencedRegion);
	DeclareAccessorCommandClass(AetObjChangeReferenceLayer, "Change Object Layer Reference", AetObj, RefPtr<AetLayer>, GetReferencedLayer, SetReferencedLayer);
	DeclareAccessorCommandClass(AetObjChangeObjReferenceParent, "Change Object Reference Parent", AetObj, RefPtr<AetObj>, GetReferencedParentObj, SetReferencedParentObj);

	DeclarePropertyCommandClass(AnimationDataChangeBlendMode, "Change Blend Mode", AnimationData, AetBlendMode, BlendMode);
	DeclarePropertyCommandClass(AnimationDataChangeUseTextureMask, "Change Use Texture Mask", AnimationData, bool, UseTextureMask);

	DeclarePropertyCommandClass(AetObjChangeMarkerName, "Change Object Marker Name", AetMarker, std::string, Name);
	DeclarePropertyCommandClass(AetObjChangeMarkerFrame, "Change Object Marker Frame", AetMarker, float, Frame);

	class AetObjAddMarker : public AetCommand
	{
		RefPtr<AetObj> ref;
		RefPtr<AetMarker> newValue;
	
	public:
		AetObjAddMarker(const RefPtr<AetObj>& ref, const RefPtr<AetMarker>& value) : ref(ref), newValue(value) {}
		void Do()	override { ref->Markers.push_back(newValue); }
		void Undo() override { ref->Markers.erase(std::find(ref->Markers.begin(), ref->Markers.end(), newValue)); }
		void Redo() override { Do(); }
		const char* GetName() override { return "Add Object Marker"; }
	};

	class AetObjDeleteMarker : public AetCommand
	{
		RefPtr<AetObj> ref;
		RefPtr<AetMarker> newValue;
		int index;

	public:
		AetObjDeleteMarker(const RefPtr<AetObj>& ref, int value) : ref(ref), index(value) {}
		void Do()	override { newValue = ref->Markers.at(index); Redo(); }
		void Undo() override { ref->Markers.insert(ref->Markers.begin() + index, newValue); }
		void Redo() override { ref->Markers.erase(ref->Markers.begin() + index); }
		const char* GetName() override { return "Delete Object Marker"; }
	};

	class AetObjMoveMarker : public AetCommand
	{
		RefPtr<AetObj> ref;
		std::tuple<int, int> newValue;

	public:
		AetObjMoveMarker(const RefPtr<AetObj>& ref, std::tuple<int, int> value) : ref(ref), newValue(value) {}
		void Do()	override { std::iter_swap(ref->Markers.begin() + std::get<0>(newValue), ref->Markers.begin() + std::get<1>(newValue)); }
		void Undo() override { std::iter_swap(ref->Markers.begin() + std::get<1>(newValue), ref->Markers.begin() + std::get<0>(newValue)); }
		void Redo() override { Do(); }
		const char* GetName() override { return "Move Object Marker"; }
	};
}

#undef DeclarePropertyCommandClass
#undef DeclareAccessorCommandClass