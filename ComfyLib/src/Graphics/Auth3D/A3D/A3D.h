#pragma once
#include "Types.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Graphics
{
	enum class A3DFormat : u32
	{
		Unknown = 0,
		Text = 'A3DA',
		Binary = 'A3DC',
		Json = 'A3DJ',
		MessagePack = 'A3DM',
	};

	struct A3DConverterData
	{
		u32 Version = 0x00000000;
	};

	struct A3DPropertyData
	{
		u32 Version = 0x00000000;
	};

	struct A3DMetadata
	{
		A3DFormat Format = A3DFormat::Unknown;
		bool Compressed16BitFloats = false;

		A3DConverterData Converter;
		A3DPropertyData Property;
		std::string FileName;
	};

	struct A3DPlayControlData
	{
		frame_t Begin = 0.0f;
		frame_t Duration = 0.0f;
		frame_t FrameRate = 0.0f;
	};

	enum class A3DKeyFrameType : u32
	{
		Frame = 0,
		FrameValue = 1,
		FrameValueCurveStart = 2,
		FrameValueCurveStartEnd = 3,
		Count,
	};

	struct A3DKeyFrame
	{
		A3DKeyFrameType Type;
		frame_t Frame;
		f32 Value;
		f32 StartTangent;
		f32 EndTangent;
	};

	enum class A3DTangentType : u32
	{
		// TODO: Fixed, Linear, Flat, Step, Slow, Fast, Spline, Clamped, Plateau, StepNext (?)
		None = 0,
		Static = 1,
		Linear = 2,
		Hermite = 3,
		Hold = 4,
		Count,
	};

	enum class A3DValueType : u32
	{
		Unknown, Float,
	};

	struct A3DRawData
	{
		A3DKeyFrameType KeyType;
		A3DValueType ValueType;
		size_t ValueListSize;
	};

	enum class A3DInfinityType : u32
	{
		// TODO: Constant, Linear, Cycle, CycleRelative, Oscillate (?)
		None = 0,
		Reverse = 1,
		Repeat = 2,
		Unknown = 3,
	};

	struct A3DProperty1D
	{
		bool Enabled;
		std::vector<A3DKeyFrame> Keys;

		A3DInfinityType PreInfinity;
		A3DInfinityType PostInfinity;

		A3DRawData RawData;
		f32 StaticValue;
		f32 Max;
		A3DTangentType Type;
	};

	struct A3DProperty3D
	{
		A3DProperty1D X, Y, Z;
	};

	struct A3DPropertyRGB
	{
		A3DProperty1D R, G, B;
	};

	struct A3DTransform
	{
		A3DProperty3D Rotation;
		A3DProperty3D Scale;
		A3DProperty3D Translation;
		A3DProperty1D Visibility;
	};

	struct A3DPoint
	{
		std::string Name;
		A3DTransform Transform;
	};

	struct A3DCurve
	{
		// TODO: C-urve V-alue, C-ontrol V-alue (?)
		std::string Name;
		A3DProperty1D CV;
	};

	struct A3DCameraViewPoint
	{
		A3DTransform Transform;
		f32 AspectRatio;
		A3DProperty1D FieldOfView;
		bool HorizontalFieldOfView;

		A3DProperty3D Roll;
	};

	struct A3DCamera
	{
		A3DTransform Transform;
		A3DTransform Interest;
		A3DCameraViewPoint ViewPoint;
	};

	struct A3DCameraAuxiliary
	{
		A3DProperty1D Exposure;
		A3DProperty1D Gamma;
		A3DProperty1D Saturate;
		A3DProperty1D AutoExposure;
	};

	struct A3DLightColor
	{
		A3DPropertyRGB Ambient;
		A3DPropertyRGB Diffuse;
		A3DPropertyRGB Specular;
		A3DPropertyRGB Incandescence;
	};

	struct A3DLight
	{
		u32 ID;
		std::string Name;
		A3DLightColor Color;
		A3DTransform Position;
		A3DTransform SpotDirection;
		std::string Type;
	};

	struct A3DFog
	{
		u32 ID;
		A3DProperty1D Density;
		A3DProperty1D Start;
		A3DProperty1D End;
		A3DPropertyRGB Diffuse;
	};

	struct A3DPostProcess
	{
		A3DLightColor LightColor;
		A3DProperty1D LensFlare;
		A3DProperty1D LensGhost;
		A3DProperty1D LensShaft;
	};

	struct A3DDepthOfField
	{
		std::string Name;
		A3DTransform Transform;
	};

	struct A3DCharacter
	{
		std::string Name;
		A3DTransform Transform;
	};

	struct A3DAuth2D
	{
		std::string Name;
	};

	struct A3DTexturePattern
	{
		std::string Name;
		std::string PatternName;
		A3DCurve* Pattern = nullptr;
		u32 PatternOffset;
	};

	struct A3DTextureTransform
	{
		std::string Name;
		A3DProperty1D CoverageU;
		A3DProperty1D CoverageV;
		A3DProperty1D RepeatU;
		A3DProperty1D RepeatV;
		A3DProperty1D Rotate;
		A3DProperty1D RotateFrame;
		A3DProperty1D OffsetU;
		A3DProperty1D OffsetV;
		A3DProperty1D TranslateFrameU;
		A3DProperty1D TranslateFrameV;
	};

	struct A3DObject
	{
		std::string Name;
		std::string UIDName;

		A3DTransform Transform;

		std::string Pat;
		u32 PatOffset;

		std::string MorphName;
		A3DCurve* Morph = nullptr;

		u32 MorphOffset;

		std::string ParentName;
		A3DObject* Parent = nullptr;

		std::vector<A3DTexturePattern> TexturePatterns;
		std::vector<A3DTextureTransform> TextureTransforms;
	};

	struct A3DNode
	{
		std::string Name;
		A3DTransform Transform;
		u32 Parent;
	};

	struct A3DObjectHRC
	{
		bool Shadow;
		std::vector<A3DNode> Nodes;
		std::string Name;
		std::string UIDName;
	};

	enum class A3DEventType : u32
	{
		Misc = 0,
		Filter = 1,
		Effect = 2,
		Sound = 3,
		Motion = 4,
		Auth2D = 5,
		Count,
	};

	struct A3DEvent
	{
		A3DEventType Type;
		std::string Name;
		frame_t Begin;
		frame_t End;
		f32 TimeReferenceScale;
		std::string Reference;
		std::array<std::string, 1> Parameters;
	};

	class A3D final : public IO::IBufferParsable
	{
	public:
		A3D() = default;
		~A3D() = default;

	public:
		A3DMetadata Metadata;
		A3DPlayControlData PlayControl;

		std::vector<A3DPoint> Points;
		std::vector<A3DCurve> Curves;

		std::vector<A3DCamera> CameraRoot;
		A3DCameraAuxiliary CameraAuxiliary;

		std::vector<A3DLight> Lights;
		std::vector<A3DFog> Fog;

		A3DPostProcess PostProcess;
		A3DDepthOfField DepthOfField;

		std::vector<A3DCharacter> Characters;
		std::vector<std::string> Motions;

		std::vector<A3DAuth2D> Auth2D;

		std::vector<A3DObject> Objects;
		std::vector<std::string> ObjectList;

		std::vector<A3DObjectHRC> ObjectsHRC;
		std::vector<std::string> ObjectHRCList;

		std::vector<A3DObjectHRC> MObjectsHRC;
		std::vector<std::string> MObjectHRCList;

		std::vector<A3DEvent> Events;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;

		// NOTE: Needs to be called every time any of the vector iterators have been invalidated
		void UpdateReferencePointers();
	};
}
