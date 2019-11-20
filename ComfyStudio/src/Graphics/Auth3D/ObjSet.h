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

	struct MaterialTexture
	{
		int32_t Field00;
		int32_t Field01;
		uint32_t TextureID;
		int32_t Field02;
		float Field03;
		float Field04;
		float Field05;
		float Field06;
		float Field07;
		float Field08;
		float Field09;
		float Field10;
		float Field11;
		float Field12;
		float Field13;
		float Field14;
		float Field15;
		float Field16;
		float Field17;
		float Field18;
		float Field19;
		float Field20;
		float Field21;
		float Field22;
		float Field23;
		float Field24;
		float Field25;
		float Field26;
		float Field27;
		float Field28;
	};

	struct Material
	{
		unk32_t Unknown[2];
		char Shader[8];

		MaterialTexture Diffuse;
		MaterialTexture Ambient;
		MaterialTexture Normal;
		MaterialTexture Specular;
		MaterialTexture ToonCurve;
		MaterialTexture Reflection;
		MaterialTexture Tangent;
		MaterialTexture Texture08;

		unk32_t Field01;
		unk32_t Field02;

		vec4 DiffuseColor;
		vec4 AmbientColor;
		vec4 SpecularColor;
		vec4 Emissioncolor;

		float Shininess;
		float Field20;
		float Field21;
		float Field22;
		float Field23;
		float Field24;

		char Name[64];

		float Field25;
		float Field26;
		float Field27;
		float Field28;
		float Field29;
		float Field30;
		float Field31;
		float Field32;
		float Field33;
		float Field34;
		float Field35;
		float Field36;
		float Field37;
		float Field38;
		float Field39;
		float Field40;
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
