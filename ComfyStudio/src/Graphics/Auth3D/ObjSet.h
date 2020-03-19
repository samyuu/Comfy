#pragma once
#include "Types.h"
#include "BoundingTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/TxpSet.h"
#include "Graphics/Direct3D/Buffer/D3D_IndexBuffer.h"
#include "Graphics/Direct3D/Buffer/D3D_VertexBuffer.h"
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

		uint32_t ReservedFlags;

		Sphere BoundingSphere;
		std::optional<Box> BoundingBox;

		uint32_t MaterialIndex;
		std::array<uint8_t, 8> UVIndices;

		std::vector<uint16_t> BoneIndices;
		uint32_t BonesPerVertex;

		PrimitiveType Primitive;

		// NOTE: IndexFormat IndexFormat; (Stored as part of the std::variant)
		std::variant<std::vector<uint8_t>, std::vector<uint16_t>, std::vector<uint32_t>> Indices;

		struct SubMeshFlags
		{
			uint32_t ReceivesShadows : 1;
			uint32_t CastsShadows : 1;
			uint32_t Transparent : 1;
		} Flags;

		UniquePtr<D3D_StaticIndexBuffer> D3D_IndexBuffer;

		// NOTE: IndexFormat wrapper around the Indices variant
		IndexFormat GetIndexFormat() const;

		std::vector<uint8_t>* GetIndicesU8();
		const std::vector<uint8_t>* GetIndicesU8() const;

		std::vector<uint16_t>* GetIndicesU16();
		const std::vector<uint16_t>* GetIndicesU16() const;

		std::vector<uint32_t>* GetIndicesU32();
		const std::vector<uint32_t>* GetIndicesU32() const;

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

		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;

		struct MeshFlags
		{
			uint32_t Unknown0 : 1;
			uint32_t FaceCameraPosition : 1;
			uint32_t Unknown1 : 1;
			uint32_t FaceCameraView : 1;
		} Flags;

		std::array<uint32_t, 7> ReservedData;
		std::array<char, 64> Name;

		struct VertexData
		{
			uint32_t Stride;
			uint32_t VertexCount;

			std::vector<vec3> Positions;
			std::vector<vec3> Normals;
			std::vector<vec4> Tangents;
			std::vector<vec4> Reserved0x3;
			std::array<std::vector<vec2>, 4> TextureCoordinates;
			std::array<std::vector<vec4>, 2> Colors;
			std::vector<vec4> BoneWeights;
			std::vector<vec4> BoneIndices;
		} VertexData;

		std::array<UniquePtr<D3D_StaticVertexBuffer>, VertexAttribute_Count> D3D_VertexBuffers;
	};

	enum class MaterialTextureType : uint32_t
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

	static constexpr std::array<const char*, static_cast<size_t>(MaterialTextureType::Count)> MaterialTextureTypeNames =
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

	enum class MaterialTextureUVIndex : uint32_t
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

	static constexpr std::array<const char*, static_cast<size_t>(MaterialTextureUVIndex::Count)> MaterialTextureUVIndexNames =
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

	enum class MaterialTextureUVTranslationType : uint32_t
	{
		None = 0,
		UV = 1,
		EnvironmentSphere = 2,
		EnvironmentCube = 3,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(MaterialTextureUVTranslationType::Count)> MaterialTextureUVTranslationTypeNames =
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
			uint32_t RepeatU : 1;
			uint32_t RepeatV : 1;
			uint32_t MirrorU : 1;
			uint32_t MirrorV : 1;

			uint32_t IgnoreAlpha : 1;
			uint32_t Blend : 5;
			uint32_t AlphaBlend : 5;

			uint32_t Border : 1;
			uint32_t ClampToEdge : 1;
			uint32_t Filter : 3;

			uint32_t MipMap : 2;
			uint32_t MipMapBias : 8;
			uint32_t AnsiFilters : 2;
		} SamplerFlags;

		Cached_TxpID TextureID;

		struct TextureDataFlags
		{
			MaterialTextureType Type : 4;
			MaterialTextureUVIndex UVIndex : 4;
			MaterialTextureUVTranslationType UVTranslationType : 3;
			uint32_t Reserved : 21;
		} TextureFlags;

		std::array<char, 8> ExShader;
		float Weight;

		mat4 TextureCoordinateMatrix;
		std::array<float, 8> ReservedData;
	};

	enum class VertexTranslationType : uint32_t
	{
		Default = 0,
		Envelope = 1,
		Morphing = 2,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(VertexTranslationType::Count)> VertexTranslationTypeNames =
	{
		"Default",
		"Envelope",
		"Morphing",
	};

	enum class ColorSourceType : uint32_t
	{
		MaterialColor = 0,
		VertexColor = 1,
		VertexMorph = 2,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(ColorSourceType::Count)> ColorSourceTypeNames =
	{
		"Material Color",
		"Vertex Color",
		"Vertex Morph",
	};

	enum class BumpMapType : uint32_t
	{
		None = 0,
		Dot = 1,
		Env = 2,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(BumpMapType::Count)> BumpMapTypeNames =
	{
		"None",
		"Dot",
		"Env",
	};

	enum class SpecularQuality : uint32_t
	{
		Low = 0,
		High = 1,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(SpecularQuality::Count)> SpecularQualityNames =
	{
		"Low",
		"Hight",
	};

	enum class AnisoDirection : uint32_t
	{
		Normal = 0,
		U = 1,
		V = 2,
		Radial = 3,
		Count,
	};

	static constexpr std::array<const char*, static_cast<size_t>(AnisoDirection::Count)> AnisoDirectionNames =
	{
		"Normal",
		"U",
		"V",
		"Radial",
	};

	enum class BlendFactor : uint32_t
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

	static constexpr std::array<const char*, static_cast<size_t>(BlendFactor::Count)> BlendFactorNames =
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

		uint32_t UsedTexturesCount;
		struct MaterialUsedTextureFlags
		{
			uint32_t Color : 1;
			uint32_t ColorA : 1;
			uint32_t ColorL1 : 1;
			uint32_t ColorL1A : 1;
			uint32_t ColorL2 : 1;
			uint32_t ColorL2A : 1;
			uint32_t Transparency : 1;
			uint32_t Specular : 1;
			uint32_t Normal : 1;
			uint32_t NormalAlt : 1;
			uint32_t Environment : 1;
			uint32_t ColorL3 : 1;
			uint32_t ColorL3A : 1;
			uint32_t Translucency : 1;
			uint32_t Unknown0 : 1;
			uint32_t OverrideIBLCubeMap : 1;
			uint32_t Reserved : 16;
		} UsedTexturesFlags;

		ShaderTypeIdentifier ShaderType;
		struct MaterialShaderFlags
		{
			VertexTranslationType VertexTranslationType : 2;
			ColorSourceType ColorSourceType : 2;

			uint32_t LambertShading : 1;
			uint32_t PhongShading : 1;
			uint32_t PerPixelShading : 1;
			uint32_t DoubleShading : 1;

			BumpMapType BumpMapType : 2;
			uint32_t Fresnel : 4;
			uint32_t LineLight : 4;

			uint32_t ReceivesShadows : 1;
			uint32_t CastsShadows : 1;

			SpecularQuality SpecularQuality : 1;
			AnisoDirection AnisoDirection : 3;

			uint32_t Reserved : 8;
		} ShaderFlags;

		std::array<MaterialTextureData, 8> Textures;

		struct MaterialBlendFlags
		{
			uint32_t AlphaTexture : 1;
			uint32_t AlphaMaterial : 1;
			uint32_t PunchThrough : 1;

			uint32_t DoubleSided : 1;
			uint32_t NormalDirectionLight : 1;

			BlendFactor SrcBlendFactor : 4;
			BlendFactor DstBlendFactor : 4;
			uint32_t BlendOperation : 3;

			uint32_t ZBias : 4;
			uint32_t NoFog : 1;

			uint32_t Unknown0 : 7;
			uint32_t Unknown1 : 1;
			uint32_t Reserved : 3;
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

		Sphere BoundingSphere;
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;
		std::optional<Skeleton> Skeleton;

	public:
		void Upload();

	private:
		void Read(FileSystem::BinaryReader& reader);
	};

	class ObjSet final : public FileSystem::IBinaryReadable, NonCopyable
	{
	public:
		ObjSet() = default;
		~ObjSet() = default;

	public:
		std::string Name;
		std::vector<TxpID> TextureIDs;
		UniquePtr<TxpSet> TxpSet;

		auto begin() { return objects.begin(); }
		auto end() { return objects.end(); }
		auto begin() const { return objects.begin(); }
		auto end() const { return objects.end(); }
		auto cbegin() const { return objects.cbegin(); }
		auto cend() const { return objects.cend(); }

		Obj& front() { return objects.front(); }
		Obj& back() { return objects.back(); }
		const Obj& front() const { return objects.front(); }
		const Obj& back() const { return objects.back(); }

		inline size_t size() const { return objects.size(); };
		inline bool empty() const { return objects.empty(); };

		inline Obj& at(size_t index) { return objects.at(index); };
		inline Obj& operator[] (size_t index) { return objects[index]; };

		inline Obj* GetObjAt(int index) { return &objects.at(index); };
		inline const Obj* GetObjAt(int index) const { return &objects[index]; };

	public:
		void Read(FileSystem::BinaryReader& reader) override;
		void UploadAll();

	public:
		static UniquePtr<ObjSet> MakeUniqueReadParseUpload(std::string_view filePath);

	private:
		std::vector<Obj> objects;
	};
}
