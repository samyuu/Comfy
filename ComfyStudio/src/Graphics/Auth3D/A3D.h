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

	struct A3DProperty1D
	{
		bool Enabled;
		std::vector<A3DKeyFrame> Keys;
		A3DRawData RawData;

		// TODO: ep_type_pre / ep_type_post
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

	struct A3DTextureTransform
	{
		std::string Name;
		// TODO: RepeatU
		A3DProperty1D TranslateFrameU;
		A3DProperty1D TranslateFrameV;
	};

	struct A3DCurveMorph
	{
		// TODO: C-urve V-alue, C-ontrol V-alue (???)
		std::string Name;
		A3DProperty1D CV;
	};

	struct A3DObject : public A3DTransform
	{
		std::string Morph;
		uint32_t MorphOffset;

		std::vector<A3DTextureTransform> TextureTransforms;
		std::string Name;
		std::string ParentName;
		std::string UIDName;
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

	struct A3DPostProcess : public A3DLightProperties
	{
		A3DProperty1D LensFlare;
		A3DProperty1D LensGhost;
		A3DProperty1D LensShaft;
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

	class A3D final : public FileSystem::IBufferParsable
	{
	public:
		A3D();
		~A3D() = default;

	public:
		A3DMetadata Metadata;
		
		A3DPlayControl PlayControl;
		
		std::vector<A3DCurveMorph> Curves;
		std::vector<A3DObject> Objects;
		std::vector<std::string> ObjectList;

		std::vector<std::string> ObjectHRCList;
		std::vector<A3DObjectHRC> ObjectsHRC;

		std::vector<A3DLight> Lights;
		A3DPostProcess PostProcess;
		std::vector<A3DCamera> CameraRoot;

	public:
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
	};
}
