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

		AetObjChangeName,
		AetObjChangeLoopStart,
		AetObjChangeLoopEnd,
		AetObjChangeStartFrame,
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
	Define_PropertyCommand(AetChangeStartFrame, "Aet Start Frame Change", Aet, float, FrameStart);
	Define_PropertyCommand(AetChangeFrameDuration, "Aet Frame Duration Change", Aet, float, FrameDuration);
	Define_PropertyCommand(AetChangeFrameRate, "Aet Frame Rate Change", Aet, float, FrameRate);
	Define_PropertyCommand(AetChangeBackgroundColor, "Aet Background Color Change", Aet, uint32_t, BackgroundColor);

	Define_AccessorCommand(AetObjChangeName, "Object Name Change", AetObj, String, GetName, SetName);
	Define_PropertyCommand(AetObjChangeLoopStart, "Object Loop Start Change", AetObj, float, LoopStart);
	Define_PropertyCommand(AetObjChangeLoopEnd, "Object Loop End Change", AetObj, float, LoopEnd);
	Define_PropertyCommand(AetObjChangeStartFrame, "Object Start Frame Change", AetObj, float, StartFrame);
	Define_PropertyCommand(AetObjChangePlaybackSpeed, "Object Playback Speed Change", AetObj, float, PlaybackSpeed);
	Define_AccessorCommand(AetObjChangeFlagsVisible, "Object Visbility Change", AetObj, bool, GetIsVisible, SetIsVisible);
	Define_AccessorCommand(AetObjChangeFlagsAudible, "Object Audibility Change", AetObj, bool, GetIsAudible, SetIsAudible);
	Define_AccessorCommand(AetObjChangeReferenceRegion, "Object Region Reference Change", AetObj, RefPtr<AetRegion>, GetReferencedRegion, SetReferencedRegion);
	Define_AccessorCommand(AetObjChangeReferenceLayer, "Object Layer Reference Change", AetObj, RefPtr<AetLayer>, GetReferencedLayer, SetReferencedLayer);
	Define_AccessorCommand(AetObjChangeObjReferenceParent, "Object Reference Parent Change", AetObj, RefPtr<AetObj>, GetReferencedParentObj, SetReferencedParentObj);

	Define_PropertyCommand(AnimationDataChangeBlendMode, "Blend Mode Change", AnimationData, AetBlendMode, BlendMode);
	Define_PropertyCommand(AnimationDataChangeUseTextureMask, "Texture Mask Change", AnimationData, bool, UseTextureMask);

	Define_PropertyCommand(AetObjChangeMarkerName, "Object Marker Name Change", AetMarker, String, Name);
	Define_PropertyCommand(AetObjChangeMarkerFrame, "Object Marker Frame Change", AetMarker, float, Frame);
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
	void Update(const RefPtr<AetMarker>& value) { newValue = value; Redo(); };
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
	void Update(const int& value) { index = value; Redo(); };
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
	void Update(const std::tuple<int, int>& value) { newValue = value; Redo(); };
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	// TODO: Consider using float KeyFrame::Frame instead of int index (?)
	// NOTE: Generic change KeyFrame value command to be used by the AetInspector
	Define_AetCommandStart(AnimationDataChangeKeyFrameValue, "Key Frame Value Change");
private:
	RefPtr<AnimationData> ref;
	std::tuple<PropertyType_Enum /* Property */, frame_t /* Frame */, float /* Value */> newValue;
	float oldValue;

	inline KeyFrameCollection& GetKeyFrames() { return ref->Properties[std::get<0>(newValue)]; };
	inline frame_t GetFrame() { return std::get<1>(newValue); };
	inline float GetValue() { return std::get<2>(newValue); };
	inline AetKeyFrame* GetKeyFrame() { return AetMgr::GetKeyFrameAt(GetKeyFrames(), GetFrame()); };

public:
	AnimationDataChangeKeyFrameValue(const RefPtr<AnimationData>& ref, std::tuple<PropertyType_Enum, frame_t, float> value) : ref(ref), newValue(value) {}

	void Do() override 
	{ 
		oldValue = GetKeyFrame()->Value; Redo();
	}
	void Undo() override 
	{ 
		GetKeyFrame()->Value = oldValue;
	}
	void Redo() override 
	{ 
		GetKeyFrame()->Value = GetValue();
	}
	void Update(const std::tuple<PropertyType_Enum, frame_t, float>& value) 
	{ 
		newValue = value; Redo(); 
	};
	const void* GetDataIdentifier() override 
	{
		return &GetKeyFrames(); 
	};
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------

	
	// ----------------------------------------------------------------------------------------------------------------------------
	// NOTE: Should only be used by the TransformTool
	Define_AetCommandStart(AnimationDataChangeTransform, "Transform");
private:
	RefPtr<AnimationData> ref;
	std::tuple<frame_t /* Frame */, vec4 /* Position and Scale */, PropertyTypeFlags /* Properties */> newValue;
	vec2 oldPosition, oldScale;

	inline frame_t GetFrame() { return std::get<0>(newValue); };
	inline vec2 GetPosition() { return vec2(std::get<1>(newValue).x, std::get<1>(newValue).y); };
	inline vec2 GetScale() { return vec2(std::get<1>(newValue).z, std::get<1>(newValue).w); };
	inline PropertyTypeFlags GetTypeFlags() { return std::get<2>(newValue); };

	inline KeyFrameCollection& GetXPositionKeyFrames() { return ref->Properties.PositionX(); };
	inline KeyFrameCollection& GetYPositionKeyFrames() { return ref->Properties.PositionY(); };
	inline KeyFrameCollection& GetXScaleKeyFrames() { return ref->Properties.ScaleX(); };
	inline KeyFrameCollection& GetYScaleKeyFrames() { return ref->Properties.ScaleY(); };

	inline AetKeyFrame* GetXPositionKeyFrame() { return !GetTypeFlags().PositionX ? nullptr : AetMgr::GetKeyFrameAt(GetXPositionKeyFrames(), GetFrame()); };
	inline AetKeyFrame* GetYPositionKeyFrame() { return !GetTypeFlags().PositionY ? nullptr : AetMgr::GetKeyFrameAt(GetYPositionKeyFrames(), GetFrame()); };
	inline AetKeyFrame* GetXScaleKeyFrame() { return !GetTypeFlags().ScaleX ? nullptr : AetMgr::GetKeyFrameAt(GetXScaleKeyFrames(), GetFrame()); };
	inline AetKeyFrame* GetYScaleKeyFrame() { return !GetTypeFlags().ScaleY ? nullptr : AetMgr::GetKeyFrameAt(GetYScaleKeyFrames(), GetFrame()); };

public:
	AnimationDataChangeTransform(const RefPtr<AnimationData>& ref, std::tuple<frame_t, vec4, PropertyTypeFlags> value) : ref(ref), newValue(value) {}

	void Do() override
	{
		auto flags = GetTypeFlags();
		oldPosition.x = flags.PositionX ? GetXPositionKeyFrame()->Value : FloatInfinity;
		oldPosition.y = flags.PositionY ? GetYPositionKeyFrame()->Value : FloatInfinity;
		oldScale.x = flags.ScaleX ? GetXScaleKeyFrame()->Value : FloatInfinity;
		oldScale.y = flags.ScaleY ? GetYScaleKeyFrame()->Value : FloatInfinity;
		Redo();
	}
	void Undo() override
	{
		auto flags = GetTypeFlags();
		if (flags.PositionX) GetXPositionKeyFrame()->Value = oldPosition.x;
		if (flags.PositionY) GetYPositionKeyFrame()->Value = oldPosition.y;
		if (flags.ScaleX) GetXScaleKeyFrame()->Value = oldScale.x;
		if (flags.ScaleY) GetYScaleKeyFrame()->Value = oldScale.y;
	}
	void Redo() override
	{
		AetKeyFrame* keyFrames[4] = { GetXPositionKeyFrame(), GetYPositionKeyFrame(), GetXScaleKeyFrame(), GetYScaleKeyFrame() };
		if (keyFrames[0]) keyFrames[0]->Value = GetPosition().x;
		if (keyFrames[1]) keyFrames[1]->Value = GetPosition().y;
		if (keyFrames[2]) keyFrames[2]->Value = GetScale().x;
		if (keyFrames[3]) keyFrames[3]->Value = GetScale().y;
	}
	void Update(const std::tuple<float, vec4, PropertyTypeFlags>& value)
	{
		newValue = value;
		Redo();
	};
	const void* GetDataIdentifier() override
	{
		return ref.get();
	};
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	// NOTE: Should only be used by the MoveTool
	Define_AetCommandStart(AnimationDataChangePosition, "Move");
private:
	RefPtr<AnimationData> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Position */, PropertyTypeFlags /* Properties */> newValue;
	vec2 oldValue;

	inline frame_t GetFrame() { return std::get<0>(newValue); };
	inline vec2 GetValue() { return std::get<1>(newValue); };
	inline PropertyTypeFlags GetTypeFlags() { return std::get<2>(newValue); };

	inline KeyFrameCollection& GetXKeyFrames() { return ref->Properties.PositionX(); };
	inline KeyFrameCollection& GetYKeyFrames() { return ref->Properties.PositionY(); };

	inline AetKeyFrame* GetXKeyFrame() { return !GetTypeFlags().PositionX ? nullptr : AetMgr::GetKeyFrameAt(GetXKeyFrames(), GetFrame()); };
	inline AetKeyFrame* GetYKeyFrame() { return !GetTypeFlags().PositionY ? nullptr : AetMgr::GetKeyFrameAt(GetYKeyFrames(), GetFrame()); };

public:
	AnimationDataChangePosition(const RefPtr<AnimationData>& ref, std::tuple<frame_t, vec2, PropertyTypeFlags> value) : ref(ref), newValue(value) {}

	void Do() override 
	{ 
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		oldValue.x = keyFrames[0] ? keyFrames[0]->Value : FloatInfinity;
		oldValue.y = keyFrames[1] ? keyFrames[1]->Value : FloatInfinity;
		Redo(); 
	}
	void Undo() override 
	{
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		if (keyFrames[0]) keyFrames[0]->Value = oldValue.x;
		if (keyFrames[1]) keyFrames[1]->Value = oldValue.y;
	}
	void Redo() override 
	{
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		if (keyFrames[0]) keyFrames[0]->Value = GetValue().x;
		if (keyFrames[1]) keyFrames[1]->Value = GetValue().y;
	}
	void Update(const std::tuple<float, vec2, PropertyTypeFlags>& value)
	{ 
		newValue = value; 
		Redo(); 
	};
	const void* GetDataIdentifier() override 
	{
		return ref.get();
	};
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------------------------------------------
	// NOTE: Should only be used by the ScaleTool
	Define_AetCommandStart(AnimationDataChangeScale, "Scale");
private:
	RefPtr<AnimationData> ref;
	std::tuple<frame_t /* Frame */, vec2 /* Scale */, PropertyTypeFlags /* Properties */> newValue;
	vec2 oldValue;

	inline frame_t GetFrame() { return std::get<0>(newValue); };
	inline vec2 GetValue() { return std::get<1>(newValue); };
	inline PropertyTypeFlags GetTypeFlags() { return std::get<2>(newValue); };

	inline KeyFrameCollection& GetXKeyFrames() { return ref->Properties.ScaleX(); };
	inline KeyFrameCollection& GetYKeyFrames() { return ref->Properties.ScaleY(); };

	inline AetKeyFrame* GetXKeyFrame() { return !GetTypeFlags().ScaleX ? nullptr : AetMgr::GetKeyFrameAt(GetXKeyFrames(), GetFrame()); };
	inline AetKeyFrame* GetYKeyFrame() { return !GetTypeFlags().ScaleY ? nullptr : AetMgr::GetKeyFrameAt(GetYKeyFrames(), GetFrame()); };

public:
	AnimationDataChangeScale(const RefPtr<AnimationData>& ref, std::tuple<frame_t, vec2, PropertyTypeFlags> value) : ref(ref), newValue(value) {}

	void Do() override
	{
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		oldValue.x = keyFrames[0] ? keyFrames[0]->Value : FloatInfinity;
		oldValue.y = keyFrames[1] ? keyFrames[1]->Value : FloatInfinity;
		Redo();
	}
	void Undo() override
	{
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		if (keyFrames[0]) keyFrames[0]->Value = oldValue.x;
		if (keyFrames[1]) keyFrames[1]->Value = oldValue.y;
	}
	void Redo() override
	{
		AetKeyFrame* keyFrames[2] = { GetXKeyFrame(), GetYKeyFrame() };
		if (keyFrames[0]) keyFrames[0]->Value = GetValue().x;
		if (keyFrames[1]) keyFrames[1]->Value = GetValue().y;
	}
	void Update(const std::tuple<float, vec2, PropertyTypeFlags>& value)
	{
		newValue = value;
		Redo();
	};
	const void* GetDataIdentifier() override
	{
		return ref.get();
	};
	Define_AetCommandEnd();
	// ----------------------------------------------------------------------------------------------------------------------------
}

#undef Define_PropertyCommand
#undef Define_AccessorCommand

#undef Define_AetCommandStart
#undef Define_AetCommandEnd
