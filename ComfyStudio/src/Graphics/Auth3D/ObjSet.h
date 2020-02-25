#pragma once
#include "Types.h"
#include "BoundingTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/TxpSet.h"
#include "Graphics/Direct3D/D3D_IndexBuffer.h"
#include "Graphics/Direct3D/D3D_VertexBuffer.h"

namespace Graphics
{
	struct Material;
	class Obj;

	struct SubMesh
	{
		Sphere BoundingSphere;
		Box BoundingBox;

		uint32_t MaterialIndex;
		uint32_t MaterialUVIndices[2];
		std::vector<uint16_t> BoneIndices;
		uint32_t UnknownPrePrimitive;
		PrimitiveType Primitive;
		uint32_t UnknownIndex;
		std::vector<uint16_t> Indices;
		uint32_t ShadowFlags;

		UniquePtr<D3D_StaticIndexBuffer> D3D_IndexBuffer;

		Material& GetMaterial(Obj& obj);
		const Material& GetMaterial(const Obj& obj) const;
	};

	struct MeshFlags
	{
		uint32_t Unknown : 1;
		uint32_t FaceCameraPosition : 1;
		uint32_t Transparent : 1;
		uint32_t FaceCameraView : 1;
	};

	struct DebugData
	{
		mutable bool RenderBoundingSphere = false;
		mutable bool WireframeOverlay = false;
	};

	struct Mesh
	{
		DebugData Debug;

		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;
		
		MeshFlags Flags;
		std::array<char, 64> Name;

		struct VertexData
		{
			uint32_t Stride;
			uint32_t VertexCount;

			std::vector<vec3> Positions;
			std::vector<vec3> Normals;
			std::vector<vec4> Tangents;
			std::vector<vec4> Attribute_0x3;
			std::array<std::vector<vec2>, 4> TextureCoordinates;
			std::array<std::vector<vec4>, 2> Colors;
			std::vector<vec4> BoneWeights;
			std::vector<vec4> BoneIndices;
		} VertexData;

		std::array<UniquePtr<D3D_StaticVertexBuffer>, VertexAttribute_Count> D3D_VertexBuffers;
	};

	struct MaterialTextureFlags
	{
		uint32_t TextureAddressMode_U_Repeat : 1;
		uint32_t TextureAddressMode_V_Repeat : 1;
		uint32_t TextureAddressMode_U_Mirror : 1;
		uint32_t TextureAddressMode_V_Mirror : 1;
		uint32_t Unknown_U_Unk0 : 1;
		uint32_t AmbientTypeFlags : 5;
		uint32_t UnknownData0 : 6;
		uint32_t UnknownData1 : 6;
		uint32_t MipMapBias : 8;
		uint32_t AnsiFilters : 2;
	};

	struct MaterialTextureTypeFlags
	{
		uint32_t ProbablyCubeMapRelated : 4;
		uint32_t Unknown0 : 4;
		uint32_t Unknown1 : 24;
	};

	struct MaterialTexture
	{
		MaterialTextureFlags Flags;
		TxpID TextureID;
		MaterialTextureTypeFlags TypeFlags;
		vec3 Field03_05;
		mat4 TextureCoordinateMatrix;
		float Reserved[8];
	};

	struct MaterialFlags
	{
		uint32_t UseDiffuseTexture : 1;
		uint32_t UseUnknown0 : 1;
		uint32_t UseAmbientTexture : 1;
		uint32_t UseUnknown1 : 1;
		uint32_t ResultsInNoTexture : 1;
		uint32_t UseUnknown2 : 1;
		uint32_t UseUnknown3 : 1;
		uint32_t UseSpecularTexture : 1;
		uint32_t UseNormalTexture : 1;
		uint32_t UseUnknown4 : 1;
		uint32_t UseUnknown5 : 1;
		uint32_t UseUnknown6 : 1;
		uint32_t UseUnknown7 : 1;
		uint32_t UseTranslucencyTexture : 1;
		uint32_t UseCubeMapReflection : 1;
		uint32_t CubeMapReflectionRelated : 16;
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

	struct MaterialShaderFlags
	{
		uint32_t Unknown0 : 4;
		uint32_t LambertShading : 1;
		uint32_t PhongShading : 1;
		uint32_t UnknownShadingMode0 : 1;
		uint32_t UnknownShadingMode1 : 1;
		uint32_t UnknownShadingMode2 : 1;
		uint32_t UnknownShadingMode3 : 1;
		uint32_t Fresnel : 4;
		uint32_t LineLight : 4;
		uint32_t Unknown2 : 2;
		SpecularQuality SpecularQuality : 1;
		AnisoDirection AnisoDirection : 3;
		uint32_t Padding : 8;
	};

	enum BlendFactor : uint32_t
	{
		BlendFactor_ZERO = 0,
		BlendFactor_ONE = 1,
		BlendFactor_SRC_COLOR = 2,
		BlendFactor_ISRC_COLOR = 3,
		BlendFactor_SRC_ALPHA = 4,
		BlendFactor_ISRC_ALPHA = 5,
		BlendFactor_DST_ALPHA = 6,
		BlendFactor_IDST_ALPHA = 7,
		BlendFactor_DST_COLOR = 8,
		BlendFactor_IDST_COLOR = 9,
		BlendFactor_Count,
	};

	enum DoubleSidedness : uint32_t
	{
		DoubleSidedness_Off = 0,
		DoubleSidedness_1_FaceLight = 1,
		DoubleSidedness_2_FaceLight = 3,
	};

	struct MaterialBlendFlags
	{
		uint32_t EnableAlphaTest : 1;
		uint32_t EnableBlend : 1;
		uint32_t OpaqueAlphaTest : 1;
		DoubleSidedness DoubleSidedness : 2;
		BlendFactor SrcBlendFactor : 4;
		BlendFactor DstBlendFactor : 4;
		uint32_t Unknown0 : 3;
		uint32_t ZBias : 4;
		uint32_t AffectsShaderUsed0 : 4;
		uint32_t AffectsShaderUsed1 : 8;
	};

	struct Material
	{
		uint32_t TextureCount;
		MaterialFlags Flags;

		std::array<char, 8> MaterialType;
		MaterialShaderFlags ShaderFlags;
		
		MaterialTexture DiffuseMap;
		MaterialTexture AmbientMap;
		MaterialTexture NormalMap;
		MaterialTexture SpecularMap;
		// NOTE: Transparency / ToonCurve
		MaterialTexture TransparencyMap;
		// NOTE: Environment / Reflection
		MaterialTexture EnvironmentMap;
		MaterialTexture TranslucencyMap;
		MaterialTexture ReservedTexture;

		MaterialBlendFlags BlendFlags;

		vec3 DiffuseColor;
		float Transparency;

		vec4 AmbientColor;

		vec3 SpecularColor;
		float Reflectivity;

		vec4 EmissionColor;

		float Shininess;
		float Intensity;

		vec4 UnknownField21_24;
		std::array<char, 64> Name;

		float BumpDepth;
		float Reserved[15];

		template <typename T>
		void IterateTextures(T func)
		{
			for (auto* texture = &DiffuseMap; texture <= &ReservedTexture; texture++)
				func(texture);
		}
		
		template <typename T>
		void IterateTextures(T func) const
		{
			for (auto* texture = &DiffuseMap; texture <= &ReservedTexture; texture++)
				func(texture);
		}

		static inline const struct Identifiers
		{
			std::array<char, 8> BLINN { "BLINN" };
			std::array<char, 8> ITEM { "ITEM" };
			std::array<char, 8> STAGE { "STAGE" };
			std::array<char, 8> SKIN { "SKIN" };
			std::array<char, 8> HAIR { "HAIR" };
			std::array<char, 8> CLOTH { "CLOTH" };
			std::array<char, 8> TIGHTS { "TIGHTS" };
			std::array<char, 8> SKY { "SKY" };
			std::array<char, 8> EYEBALL { "EYEBALL" };
			std::array<char, 8> EYELENS { "EYELENS" };
			std::array<char, 8> GLASEYE { "GLASEYE" };
			std::array<char, 8> WATER01 { "WATER01" };
			std::array<char, 8> WATER02 { "WATER02" };
			std::array<char, 8> FLOOR { "FLOOR" };
		} Identifiers;
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
		std::vector<Bone> Bones;
	};

	class Obj
	{
		friend class ObjSet;

	public:
		DebugData Debug;

		std::string Name;
		ObjID ID;

		Sphere BoundingSphere;
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;
		Skeleton Skeleton;

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
