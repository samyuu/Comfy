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
		uint32_t Flags;

		Sphere BoundingSphere;
		std::optional<Box> BoundingBox;

		uint32_t MaterialIndex;
		std::array<uint8_t, 8> UVIndices;

		std::vector<uint16_t> BoneIndices;
		uint32_t BonesPerVertex;

		PrimitiveType Primitive;

		// NOTE: IndexFormat IndexFormat; (Stored as part of the std::variant)
		std::variant<std::vector<uint8_t>, std::vector<uint16_t>, std::vector<uint32_t>> Indices;

		struct SubMeshShadowFlags
		{
			uint32_t ReceivesShadows : 1;
			uint32_t CastsShadows : 1;
		} ShadowFlags;

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

		// NOTE: Convenient helper
		Material& GetMaterial(Obj& obj);
		const Material& GetMaterial(const Obj& obj) const;
	};

	struct Mesh
	{
		mutable DebugData Debug;

		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;

		struct MeshFlags
		{
			uint32_t Unknown : 1;
			uint32_t FaceCameraPosition : 1;
			uint32_t Transparent : 1;
			uint32_t FaceCameraView : 1;
		} Flags;

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

	struct MaterialTextureData
	{
		enum TextureType
		{
			TextureType_Diffuse = 0,
			TextureType_Ambient = 1,
			TextureType_Normal = 2,
			TextureType_Specular = 3,
			TextureType_Transparency = 4,
			TextureType_Environment = 5,
			TextureType_Translucency = 6,
			TextureType_ReservedTexture = 7,
			TextureType_Count
		};

		static constexpr std::array<const char*, TextureType_Count> TextureTypeNames =
		{
			"Diffuse",
			"Ambient",
			"Normal",
			"Specular",
			"Transparency",
			"Environment",
			"Translucency",
			"Reserved"
		};

		struct TextureDataFlags
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
		} TextureFlags;

		Cached_TxpID TextureID;

		struct ShaderDataFlags
		{
			uint32_t TextureType : 4;
			uint32_t UVIndex : 4;
			uint32_t TextureCoordinateTranslation : 3;
			uint32_t Reserved : 21;
		} ShaderFlags;

		std::array<char, 8> ExShader;
		float Weight;

		mat4 TextureCoordinateMatrix;
		std::array<float, 8> ReservedData;
	};

	enum SpecularQuality : uint32_t
	{
		SpecularQuality_Low = 0,
		SpecularQuality_High = 1,
	};

	enum AnisoDirection : uint32_t
	{
		AnisoDirection_Normal = 0,
		AnisoDirection_U = 1,
		AnisoDirection_V = 2,
		AnisoDirection_Radial = 3,
	};

	enum BlendFactor : uint32_t
	{
		BlendFactor_Zero = 0,
		BlendFactor_One = 1,
		BlendFactor_SrcColor = 2,
		BlendFactor_ISrcColor = 3,
		BlendFactor_SrcAlpha = 4,
		BlendFactor_ISrcAlpha = 5,
		BlendFactor_DstAlpha = 6,
		BlendFactor_IDstAlpha = 7,
		BlendFactor_DstColor = 8,
		BlendFactor_IDstColor = 9,
		BlendFactor_Count,
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
		struct MaterialTextureFlags
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
			uint32_t Unknown1 : 1;
			uint32_t Reserved : 16;
		} UsedTexturesFlags;

		ShaderTypeIdentifier ShaderType;
		struct MaterialShaderFlags
		{
			uint32_t VertexTransType : 2;
			uint32_t ColorSource : 2;

			uint32_t LambertShading : 1;
			uint32_t PhongShading : 1;
			uint32_t PerPixelShading : 1;
			uint32_t DoubleShading : 1;

			uint32_t BumpMapType : 2;
			uint32_t Fresnel : 4;
			uint32_t LineLight : 4;

			uint32_t ReceivesShadows : 1;
			uint32_t CastsShadows : 1;
			
			SpecularQuality SpecularQuality : 1;
			AnisoDirection AnisoDirection : 3;

			uint32_t Reserved : 8;
		} ShaderFlags;

		union
		{
			struct TextureTypesData
			{
				MaterialTextureData Diffuse;
				MaterialTextureData Ambient;
				MaterialTextureData Normal;
				MaterialTextureData Specular;
				// NOTE: Transparency / ToonCurve
				MaterialTextureData Transparency;
				// NOTE: Environment / Reflection
				MaterialTextureData Environment;
				MaterialTextureData Translucency;
				MaterialTextureData ReservedTexture;
			} TextureData;
			std::array<MaterialTextureData, MaterialTextureData::TextureType_Count> TextureDataArray;
		};

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

			uint32_t Reserved : 11;
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
