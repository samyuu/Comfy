#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"

namespace Graphics
{
	enum class A3DFormat : uint32_t
	{
		Unknown = 0,
		Text = 'A3DA',
		Binary = 'A3DC',
		Json = 'A3DJ',
		MessagePack = 'A3DM',
	};

	struct A3DConverter
	{
		uint32_t Version = 0x00000000;
	};

	struct A3DProperty
	{
		uint32_t Version = 0x00000000;
	};

	struct A3DMetadata
	{
		A3DFormat Format = A3DFormat::Unknown;
		bool Compressed16BitFloats = false;

		A3DConverter Converter;
		A3DProperty Property;
		std::string FileName;
	};

	struct A3DPlayControl
	{
		frame_t Begin = 0.0f;
		frame_t Duration = 0.0f;
		frame_t FrameRate = 0.0f;
	};

	enum class A3DKeyFrameType : uint32_t
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
		float Value;
		float StartCurve;
		float EndCurve;
	};

	enum class A3DInterpolationType : uint32_t
	{
		None = 0,
		Static = 1,
		Linear = 2,
		Hermit = 3,
		Hold = 4,
		Count,
	};

	enum class A3DValueType : uint32_t
	{
		Unknown, Float,
	};

	struct A3DRawData
	{
		A3DKeyFrameType KeyType;
		A3DValueType ValueType;
		size_t ValueListSize;
	};

	enum class EPType : uint32_t
	{
		// TODO:
		None = 0,
		Reverse = 1,
		Repeat = 2,
		Unknown = 3,
	};

	struct A3DProperty1D
	{
		bool Enabled;
		std::vector<A3DKeyFrame> Keys;

		EPType EPTypePre;
		EPType EPTypePost;
		
		A3DRawData RawData;
		float StaticValue;
		float Max;
		A3DInterpolationType Type;
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

	struct A3DPoint : public A3DTransform
	{
		std::string Name;
	};

	struct A3DCurve
	{
		// TODO: C-urve V-alue, C-ontrol V-alue (???)
		std::string Name;
		A3DProperty1D CV;
	};

	struct A3DCameraViewPoint : public A3DTransform
	{
		float AspectRatio;
		A3DProperty1D FieldOfView;
		bool HorizontalFieldOfView;

		A3DProperty3D Roll;
	};

	struct A3DCamera : public A3DTransform
	{
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

	struct A3DLightProperties
	{
		A3DPropertyRGB Ambient;
		A3DPropertyRGB Diffuse;
		A3DPropertyRGB Specular;
		A3DPropertyRGB Incandescence;
	};

	struct A3DLight : public A3DLightProperties
	{
		uint32_t ID;
		std::string Name;
		A3DTransform Position;
		A3DTransform SpotDirection;
		std::string Type;
	};

	struct A3DFog
	{
		uint32_t ID;
		A3DProperty1D Density;
		A3DProperty1D Start;
		A3DProperty1D End;
		A3DPropertyRGB Diffuse;
	};

	struct A3DPostProcess : public A3DLightProperties
	{
		A3DProperty1D LensFlare;
		A3DProperty1D LensGhost;
		A3DProperty1D LensShaft;
	};

	struct A3DDepthOfField : public A3DTransform
	{
		std::string Name;
	};

	struct A3DCharacter : public A3DTransform
	{
		std::string Name;
	};

	struct A3DAuth2D
	{
		std::string Name;
	};

	// TODO: Texture Pat(tern) (?)
	struct A3DTexturePat
	{
		std::string Name;
		std::string Pat;
		uint32_t PatOffset;
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

	struct A3DObject : public A3DTransform
	{
		std::string Name;
		std::string UIDName;

		std::string Pat;
		uint32_t PatOffset;

		std::string Morph;
		uint32_t MorphOffset;

		std::string ParentName;

		std::vector<A3DTexturePat> TexturePats;
		std::vector<A3DTextureTransform> TextureTransforms;
	};

	struct A3DNode : public A3DTransform
	{
		std::string Name;
		uint32_t Parent;
	};

	struct A3DObjectHRC
	{
		bool Shadow;
		std::vector<A3DNode> Nodes;
		std::string Name;
		std::string UIDName;
	};

	enum class A3DEventType : uint32_t
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
		float TimeReferenceScale;
		std::string Reference;
		std::array<std::string, 1> Parameters;
	};

	class A3D final : public FileSystem::IBufferParsable
	{
	public:
		A3D();
		~A3D() = default;

	public:
		A3DMetadata Metadata;
		A3DPlayControl PlayControl;
		
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
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
	};
}
