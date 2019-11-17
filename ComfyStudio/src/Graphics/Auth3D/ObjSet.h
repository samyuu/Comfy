#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/Direct3D/D3D_IndexBuffer.h"
#include "Graphics/Direct3D/D3D_VertexBuffer.h"
#include "Graphics/Direct3D/D3D_InputLayout.h"

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

	class SubMesh
	{
	public:
		Sphere BoundingSphere;
		uint32_t MaterialIndex;
		uint32_t MaterialUVIndices[2];
		PrimitiveType Primitive;
		std::vector<uint16_t> Indices;
		Box BoundingBox;

		UniquePtr<D3D_StaticIndexBuffer> GraphicsIndexBuffer;
	};

	union VertexAttributeTypes
	{
		struct
		{
			uint32_t Position : 1;
			uint32_t Normal : 1;
			uint32_t Tangent : 1;
			uint32_t TextureCoordinate0 : 1;
			uint32_t TextureCoordinate1 : 1;
			uint32_t Color : 1;
			uint32_t BoneWeight : 1;
			uint32_t BoneIndex : 1;
		};
		uint32_t AllBits;
	};

	struct VertexData
	{
		uint32_t Stride;
		std::vector<vec3> Positions;
		std::vector<vec3> Normals;
		std::vector<vec4> Tangents;
	};

	struct GraphicsVertexBuffers
	{
		UniquePtr<D3D_InputLayout> InputLayout;
		UniquePtr<D3D_StaticVertexBuffer> PositionBuffer;
		UniquePtr<D3D_StaticVertexBuffer> NormalBuffer;
	};

	class Mesh
	{
	public:
		Sphere BoundingSphere;
		std::vector<RefPtr<SubMesh>> SubMeshes;
		std::string Name;
		VertexAttributeTypes VertexAttributes;
		VertexData VertexData;

		GraphicsVertexBuffers GraphicsBuffers;
	};

	class Material
	{
	public:
		std::string Name;
	};

	class Obj
	{
		friend class ObjSet;

	public:
		Obj() = default;
		Obj(const Obj&) = delete;
		~Obj() = default;

		Obj& operator=(const Obj&) = delete;

	public:
		std::string Name;
		uint32_t ID;

		Sphere BoundingSphere;
		std::vector<RefPtr<Mesh>> Meshes;
		std::vector<RefPtr<Material>> Materials;

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

		auto begin() { return objects.begin(); }
		auto end() { return objects.end(); }
		auto begin() const { return objects.begin(); }
		auto end() const { return objects.end(); }
		auto cbegin() const { return objects.cbegin(); }
		auto cend() const { return objects.cend(); }

		RefPtr<Obj>& front() { return objects.front(); }
		RefPtr<Obj>& back() { return objects.back(); }
		const RefPtr<Obj>& front() const { return objects.front(); }
		const RefPtr<Obj>& back() const { return objects.back(); }

		inline size_t size() const { return objects.size(); };

		inline RefPtr<Obj>& at(size_t index) { return objects.at(index); };
		inline RefPtr<Obj>& operator[] (size_t index) { return objects[index]; };

		inline Obj* GetObjAt(int index) { return objects.at(index).get(); };
		inline const Obj* GetObjAt(int index) const { return objects[index].get(); };

	public:
		virtual void Read(FileSystem::BinaryReader& reader) override;
		void UploadAll();

	private:
		template <class T>
		void InitializeBufferIfAttribute(bool hasAttribute, UniquePtr<D3D_StaticVertexBuffer>& vertexBuffer, std::vector<T>& vertexData, Mesh* mesh, const char* name);

	private:
		std::vector<RefPtr<Obj>> objects;
	};
}