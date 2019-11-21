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

	struct SubMesh
	{
		Sphere BoundingSphere;
		uint32_t MaterialIndex;
		uint32_t MaterialUVIndices[2];
		PrimitiveType Primitive;
		std::vector<uint16_t> Indices;
		Box BoundingBox;

		UniquePtr<D3D_StaticIndexBuffer> GraphicsIndexBuffer;
	};

	struct Mesh
	{
		Sphere BoundingSphere;
		std::vector<SubMesh> SubMeshes;
		VertexAttributeFlags AttributeFlags;
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

	struct MaterialTextureFlags0
	{
		uint32_t Unknown0 : 10;
		uint32_t Fresnel : 4;
		uint32_t LineLight : 4;
		uint32_t Unknown1 : 2;
		uint32_t SpecularQuality : 1;
		uint32_t AnisoDirection : 3;
	};

	struct MaterialTextureFlags1
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

	struct MaterialTextureFlags2
	{
		uint32_t ProbablyCubeMapRelated : 4;
		uint32_t Unknown : 4;
	};

	struct MaterialTexture
	{
		MaterialTextureFlags0 Flags0;
		MaterialTextureFlags1 Flags1;
		int32_t TextureID;
		MaterialTextureFlags2 Flags2;
		vec3 Field03_05;
		mat4 TextureCoordinateMatrix;
		float Reserved[7];
	};

	struct MaterialFlags
	{
		uint32_t IsTransparent : 1;
		uint32_t TransparentType : 2;
		uint32_t Unknown0 : 5;
		uint32_t Unknown1 : 7;
		uint32_t UseCubeMapReflection : 1;
		uint32_t Reserved : 16;
	};

	struct MaterialBlendFlags
	{
		enum BlendFactor : uint32_t
		{
			ZERO = 0,
			ONE = 1,
			SRC_COLOR = 2,
			ISRC_COLOR = 3,
			SRC_ALPHA = 4,
			ISRC_ALPHA = 5,
			DST_ALPHA = 6,
			IDST_ALPHA = 7,
			DST_COLOR = 8,
			IDST_COLOR = 9,
		};

		uint32_t EnableAlphaTest : 1;
		uint32_t EnableBlend : 1;
		uint32_t OtherBlendingFlag : 1;
		uint32_t DoubleSided : 2;
		BlendFactor SrcBlendFactor : 4;
		BlendFactor DstBlendFactor : 4;
		uint32_t Unknown0 : 3;
		uint32_t ZBias : 4;
		uint32_t Unknown1 : 4;
		uint32_t Unknown2 : 8;
	};

	struct Material
	{
		unk32_t Unknown0;
		MaterialFlags Flags;
		char Shader[8];
		MaterialTexture Diffuse;
		MaterialTexture Ambient;
		MaterialTexture Normal;
		MaterialTexture Specular;
		MaterialTexture ToonCurve;
		MaterialTexture Reflection;
		MaterialTexture Tangent;
		MaterialTexture UnknownTexture;
		int32_t Unknown1;
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

		inline Obj& at(size_t index) { return objects.at(index); };
		inline Obj& operator[] (size_t index) { return objects[index]; };

		inline Obj* GetObjAt(int index) { return &objects.at(index); };
		inline const Obj* GetObjAt(int index) const { return &objects[index]; };

	public:
		virtual void Read(FileSystem::BinaryReader& reader) override;
		void UploadAll();

	private:
		std::vector<Obj> objects;
	};
}
