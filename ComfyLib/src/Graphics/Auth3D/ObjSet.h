#pragma once
#include "Types.h"
#include "BoundingTypes.h"
#include "Graphics/TexSet.h"
#include "IO/Stream/FileInterfaces.h"
#include <optional>
#include <variant>

namespace Comfy::Graphics
{
	struct DebugData
	{
		bool RenderBoundingSphere = false;
		bool WireframeOverlay = false;
		bool UseDebugMaterial = false;
	};

	struct Material;
	class Obj;

	struct SubMesh
	{
		mutable DebugData Debug;

		u32 ReservedFlags;
		Sphere BoundingSphere;
		std::optional<Box> BoundingBox;

		u32 MaterialIndex;
		std::array<u8, 8> UVIndices;

		std::vector<u16> BoneIndices;
		u32 BonesPerVertex;

		PrimitiveType Primitive;

		// NOTE: IndexFormat IndexFormat; (Stored as part of the std::variant)
		std::variant<std::vector<u8>, std::vector<u16>, std::vector<u32>> Indices;

		struct SubMeshFlags
		{
			u32 ReceivesShadows : 1;
			u32 CastsShadows : 1;
			u32 Transparent : 1;
		} Flags;

		InternallyManagedGPUResource GPU_IndexBuffer;

		// NOTE: IndexFormat wrapper around the Indices variant
		IndexFormat GetIndexFormat() const;

		std::vector<u8>* GetIndicesU8();
		const std::vector<u8>* GetIndicesU8() const;

		std::vector<u16>* GetIndicesU16();
		const std::vector<u16>* GetIndicesU16() const;

		std::vector<u32>* GetIndicesU32();
		const std::vector<u32>* GetIndicesU32() const;

		// NOTE: Index count of the current valid index format
		const size_t GetIndexCount() const;

		// NOTE: Raw byte view for interfacing with generic data
		const void* GetRawIndices() const;
		// NOTE: Raw byte size for interfacing with generic data
		size_t GetRawIndicesByteSize() const;
	};

	struct Mesh
	{
		mutable DebugData Debug;

		u32 ReservedFlags;
		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;

		struct MeshFlags
		{
			u32 Unknown0 : 1;
			u32 FaceCameraPosition : 1;
			u32 Unknown1 : 1;
			u32 FaceCameraView : 1;
		} Flags;

		std::array<u32, 7> ReservedData;
		std::array<char, 64> Name;

		struct VertexData
		{
			u32 Stride;
			u32 VertexCount;

			// TODO: Switch to std::uniuqe_ptr<T[]>, get rid of unused UV[2], UV[3] and Color[1] (?)
			// TODO: Either convert to on load or std::varient support f16 / i16
			std::vector<vec3> Positions;
			std::vector<vec3> Normals;
			std::vector<vec4> Tangents;
			std::array<std::vector<vec2>, 4> TextureCoordinates;
			std::array<std::vector<vec4>, 2> Colors;
			std::vector<vec4> BoneWeights;
			std::vector<vec4> BoneIndices;
		} VertexData;

		std::array<InternallyManagedGPUResource, VertexAttribute_Count> GPU_VertexBuffers;
	};

	enum class MaterialTextureType : u32
	{
		None = 0,
		ColorMap = 1,
		NormalMap = 2,
		SpecularMap = 3,
		HeightMap = 4,
		ReflectionMap = 5,
		TranslucencyMap = 6,
		TransparencyMap = 7,
		EnvironmentMapSphere = 8,
		EnvironmentMapCube = 9,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<MaterialTextureType>()> MaterialTextureTypeNames =
	{
		"None",
		"Color Map",
		"Normal Map",
		"Specular Map",
		"Height Map",
		"Reflection Map",
		"Translucency Map",
		"Transparency Map",
		"Environment Map (Sphere)",
		"Environment Map (Cube)",
	};

	enum class MaterialTextureUVIndex : u32
	{
		Index_0 = 0,
		Index_1 = 1,
		Index_3 = 3,
		Index_4 = 4,
		Index_5 = 5,
		Index_6 = 6,
		Index_7 = 7,
		None = 0xF,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<MaterialTextureUVIndex>()> MaterialTextureUVIndexNames =
	{
		"Index 0",
		"Index 1",
		"Index 2",
		"Index 3",
		"Index 4",
		"Index 5",
		"Index 6",
		"Index 7",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		"None",
	};

	enum class MaterialTextureUVTranslationType : u32
	{
		None = 0,
		UV = 1,
		EnvironmentSphere = 2,
		EnvironmentCube = 3,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<MaterialTextureUVTranslationType>()> MaterialTextureUVTranslationTypeNames =
	{
		"None",
		"UV",
		"Environment Sphere",
		"Environment Cube",
	};

	struct MaterialTextureData
	{
		struct TextureSamplerFlags
		{
			u32 RepeatU : 1;
			u32 RepeatV : 1;
			u32 MirrorU : 1;
			u32 MirrorV : 1;

			u32 IgnoreAlpha : 1;
			u32 Blend : 5;
			u32 AlphaBlend : 5;

			u32 Border : 1;
			u32 ClampToEdge : 1;
			u32 Filter : 3;

			u32 MipMap : 2;
			u32 MipMapBias : 8;
			u32 AnsiFilters : 2;
		} SamplerFlags;

		Cached_TexID TextureID;

		struct TextureDataFlags
		{
			MaterialTextureType Type : 4;
			MaterialTextureUVIndex UVIndex : 4;
			MaterialTextureUVTranslationType UVTranslationType : 3;
			u32 Reserved : 21;
		} TextureFlags;

		std::array<char, 8> ExShader;
		float Weight;

		mat4 TextureCoordinateMatrix;
		std::array<float, 8> ReservedData;
	};

	enum class VertexTranslationType : u32
	{
		Default = 0,
		Envelope = 1,
		Morphing = 2,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<VertexTranslationType>()> VertexTranslationTypeNames =
	{
		"Default",
		"Envelope",
		"Morphing",
	};

	enum class ColorSourceType : u32
	{
		MaterialColor = 0,
		VertexColor = 1,
		VertexMorph = 2,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<ColorSourceType>()> ColorSourceTypeNames =
	{
		"Material Color",
		"Vertex Color",
		"Vertex Morph",
	};

	enum class BumpMapType : u32
	{
		None = 0,
		Dot = 1,
		Env = 2,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<BumpMapType>()> BumpMapTypeNames =
	{
		"None",
		"Dot",
		"Env",
	};

	enum class SpecularQuality : u32
	{
		Low = 0,
		High = 1,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<SpecularQuality>()> SpecularQualityNames =
	{
		"Low",
		"Hight",
	};

	enum class AnisoDirection : u32
	{
		Normal = 0,
		U = 1,
		V = 2,
		Radial = 3,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<AnisoDirection>()> AnisoDirectionNames =
	{
		"Normal",
		"U",
		"V",
		"Radial",
	};

	enum class BlendFactor : u32
	{
		Zero = 0,
		One = 1,
		SrcColor = 2,
		InverseSrcColor = 3,
		SrcAlpha = 4,
		InverseSrcAlpha = 5,
		DstAlpha = 6,
		InverseDstAlpha = 7,
		DstColor = 8,
		InverseDstColor = 9,
		Count,
	};

	static constexpr std::array<const char*, EnumCount<BlendFactor>()> BlendFactorNames =
	{
		"Zero",
		"One",
		"Source Color",
		"Inverse Source Color",
		"Source Alpha",
		"Inverse Source Alpha",
		"Destination Alpha",
		"Inverse Destination Alpha",
		"Destination Color",
		"Inverse Destination Color",
	};

	struct Material
	{
		using ShaderTypeIdentifier = std::array<char, 8>;

		struct ShaderIdentifiers
		{
			static constexpr ShaderTypeIdentifier Blinn = { "BLINN" };
			static constexpr ShaderTypeIdentifier Item = { "ITEM" };
			static constexpr ShaderTypeIdentifier Stage = { "STAGE" };
			static constexpr ShaderTypeIdentifier Skin = { "SKIN" };
			static constexpr ShaderTypeIdentifier Hair = { "HAIR" };
			static constexpr ShaderTypeIdentifier Cloth = { "CLOTH" };
			static constexpr ShaderTypeIdentifier Tights = { "TIGHTS" };
			static constexpr ShaderTypeIdentifier Sky = { "SKY" };
			static constexpr ShaderTypeIdentifier EyeBall = { "EYEBALL" };
			static constexpr ShaderTypeIdentifier EyeLens = { "EYELENS" };
			static constexpr ShaderTypeIdentifier GlassEye = { "GLASEYE" };
			static constexpr ShaderTypeIdentifier Simple = { "SIMPLE" };
			static constexpr ShaderTypeIdentifier Water01 = { "WATER01" };
			static constexpr ShaderTypeIdentifier Water02 = { "WATER02" };
			static constexpr ShaderTypeIdentifier Floor = { "FLOOR" };
		};

		mutable DebugData Debug;

		u32 UsedTexturesCount;
		struct MaterialUsedTextureFlags
		{
			u32 Color : 1;
			u32 ColorA : 1;
			u32 ColorL1 : 1;
			u32 ColorL1A : 1;
			u32 ColorL2 : 1;
			u32 ColorL2A : 1;
			u32 Transparency : 1;
			u32 Specular : 1;
			u32 Normal : 1;
			u32 NormalAlt : 1;
			u32 Environment : 1;
			u32 ColorL3 : 1;
			u32 ColorL3A : 1;
			u32 Translucency : 1;
			u32 Unknown0 : 1;
			u32 OverrideIBLCubeMap : 1;
			u32 Reserved : 16;
		} UsedTexturesFlags;

		ShaderTypeIdentifier ShaderType;
		struct MaterialShaderFlags
		{
			VertexTranslationType VertexTranslationType : 2;
			ColorSourceType ColorSourceType : 2;

			u32 LambertShading : 1;
			u32 PhongShading : 1;
			u32 PerPixelShading : 1;
			u32 DoubleShading : 1;

			BumpMapType BumpMapType : 2;
			u32 Fresnel : 4;
			u32 LineLight : 4;

			u32 ReceivesShadows : 1;
			u32 CastsShadows : 1;

			SpecularQuality SpecularQuality : 1;
			AnisoDirection AnisoDirection : 3;

			u32 Reserved : 8;
		} ShaderFlags;

		std::array<MaterialTextureData, 8> Textures;

		struct MaterialBlendFlags
		{
			u32 AlphaTexture : 1;
			u32 AlphaMaterial : 1;
			u32 PunchThrough : 1;

			u32 DoubleSided : 1;
			u32 NormalDirectionLight : 1;

			BlendFactor SrcBlendFactor : 4;
			BlendFactor DstBlendFactor : 4;
			u32 BlendOperation : 3;

			u32 ZBias : 4;
			u32 NoFog : 1;

			u32 Unknown0 : 7;
			u32 ForceOpaque : 1;
			u32 Reserved : 3;
		} BlendFlags;

		struct MaterialColor
		{
			vec3 Diffuse;
			float Transparency;
			vec4 Ambient;
			vec3 Specular;
			float Reflectivity;
			vec4 Emission;
			float Shininess;
			float Intensity;
		} Color;

		Sphere ReservedSphere;
		std::array<char, 64> Name;

		float BumpDepth;
		std::array<float, 15> ReservedData;
	};

	struct Bone
	{
		BoneID ID;
		mat4 Transform;
		std::string Name;
		BoneID ParentID;
	};

	struct Skeleton
	{
		static constexpr size_t MaxBoneCount = 192;
		std::vector<Bone> Bones;
	};

	class Obj
	{
		friend class ObjSet;

	public:
		mutable DebugData Debug;

		std::string Name;
		ObjID ID;

		u32 Version;
		u32 ReservedFlags;
		Sphere BoundingSphere;
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;
		std::optional<Skeleton> Skeleton;

	private:
		void Read(IO::StreamReader& reader);
	};

	class ObjSet final : public IO::IStreamReadable, NonCopyable
	{
	public:
		ObjSet() = default;
		~ObjSet() = default;

	public:
		std::string Name;
		std::vector<TexID> TextureIDs;
		std::unique_ptr<TexSet> TexSet;
		std::vector<Obj> Objects;

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
	};
}
