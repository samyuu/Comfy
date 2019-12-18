#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/TxpSet.h"
#include "Graphics/Direct3D/D3D_IndexBuffer.h"
#include "Graphics/Direct3D/D3D_VertexBuffer.h"

namespace Graphics
{
	struct Sphere
	{
		vec3 Center;
		float Radius;
	};

	struct Box
	{
		vec3 Center;
		vec3 Size;
	};

	struct Material;
	class Obj;

	struct SubMesh
	{
		Sphere BoundingSphere;
		uint32_t MaterialIndex;
		uint32_t MaterialUVIndices[2];
		PrimitiveType Primitive;
		std::vector<uint16_t> Indices;
		Box BoundingBox;

		UniquePtr<D3D_StaticIndexBuffer> GraphicsIndexBuffer;

		Material& GetMaterial(Obj& obj);
		const Material& GetMaterial(const Obj& obj) const;
	};

	struct MeshFlags
	{
		uint32_t Unknown : 1;
		uint32_t FaceCamera : 1;
		uint32_t Transparent : 1;
	};

	struct Mesh
	{
		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;
		
		MeshFlags Flags;
		char Name[64];

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

		std::array<UniquePtr<D3D_StaticVertexBuffer>, VertexAttribute_Count> GraphicsAttributeBuffers;
	};

	struct MaterialTextureFlags
	{
		uint32_t TextureAddressMode_U_Repeat : 1;
		uint32_t TextureAddressMode_V_Repeat : 1;
		uint32_t TextureAddressMode_U_Mirror : 1;
		uint32_t TextureAddressMode_V_Mirror : 1;
		uint32_t Unknown_U_Unk0 : 1;
		uint32_t Unknown_V_Unk0 : 1;
		uint32_t Unknown_U_Unk1 : 1;
		uint32_t Unknown_V_Unk1 : 1;
		uint32_t UnknownData0 : 8;
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
		int32_t TextureID;
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
		uint32_t UseTangentTexture : 1;
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
		uint32_t LightingModel_Lambert : 1;
		uint32_t LightingModel_Phong : 1;
		uint32_t LightingModel_Unknown0 : 1;
		uint32_t LightingModel_Unknown1 : 1;
		uint32_t LightingModel_Unknown2 : 1;
		uint32_t LightingModel_Unknown3 : 1;
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
		uint32_t OtherBlendingFlag : 1;
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
		std::array<char, 8> Shader;
		MaterialShaderFlags ShaderFlags;
		MaterialTexture Diffuse;
		MaterialTexture Ambient;
		MaterialTexture Normal;
		MaterialTexture Specular;
		MaterialTexture ToonCurve;
		MaterialTexture Reflection;
		MaterialTexture Tangent;
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
		char Name[64];
		float BumpDepth;
		float Reserved[15];

		template <typename T>
		void IterateMaterialTextures(T func) const
		{
			for (auto* texture = &Diffuse; texture <= &ReservedTexture; texture++)
				func(texture);
		}
	};

	class Obj
	{
		friend class ObjSet;

	public:
		std::string Name;
		uint32_t ID;

		Sphere BoundingSphere;
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;

	private:
		void Read(FileSystem::BinaryReader& reader);
	};

	class ObjSet : public FileSystem::IBinaryReadable
	{
	public:
		ObjSet() = default;
		ObjSet(const ObjSet&) = delete;
		~ObjSet() = default;

		ObjSet& operator=(const ObjSet&) = delete;

	public:
		std::string Name;
		std::vector<uint32_t> TextureIDs;
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
		virtual void Read(FileSystem::BinaryReader& reader) override;
		void UploadAll();

	public:
		static UniquePtr<ObjSet> MakeUniqueReadParseUpload(std::string_view filePath);

	private:
		std::vector<Obj> objects;
	};
}
