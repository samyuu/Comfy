#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/RenderCommand.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexArray.h"
#include "Core/CoreTypes.h"

namespace FileSystem
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
		Graphics::PrimitiveType Primitive;
		Vector<uint16_t> Indices;
		Box BoundingBox;

		RefPtr<Graphics::IndexBuffer> GraphicsIndexBuffer;
	};

	union VertexAttributeTypes
	{
		struct
		{
			uint32_t Position : 1;
			uint32_t Normal : 1;
			uint32_t Tangents : 1;
			uint32_t UVChannel0 : 1;
			uint32_t UVChannel1 : 1;
			uint32_t Color : 1;
			uint32_t BoneWeight : 1;
			uint32_t BoneIndex : 1;
		};
		uint32_t AllBits;
	};

	struct VertexData
	{
		uint32_t Stride;
		Vector<vec3> Positions;
		Vector<vec3> Normals;
		Vector<vec4> Tangents;
	};

	struct GraphicsVertexBuffers
	{
		RefPtr<Graphics::VertexArray> VertexArray;
		RefPtr<Graphics::VertexBuffer> PositionBuffer;
		RefPtr<Graphics::VertexBuffer> NormalBuffer;
	};

	class Mesh
	{
	public:
		Sphere BoundingSphere;
		Vector<RefPtr<SubMesh>> SubMeshes;
		String Name;
		VertexAttributeTypes VertexAttributes;
		VertexData VertexData;

		GraphicsVertexBuffers GraphicsBuffers;
	};

	class Material
	{
	public:
		String Name;
	};

	class Obj
	{
		friend class ObjSet;

	public:
		Obj() = default;
		Obj(Obj& other) = delete;
		Obj& operator= (Obj& other) = delete;
		~Obj() = default;

	public:
		String Name;
		uint32_t ID;

		Sphere BoundingSphere;
		Vector<RefPtr<Mesh>> Meshes;
		Vector<RefPtr<Material>> Materials;

	private:
		void Read(BinaryReader& reader);
	};

	using ObjCollection = Vector<RefPtr<Obj>>;
	using ObjIterator = ObjCollection::iterator;
	using ConstObjIterator = ObjCollection::const_iterator;

	class ObjSet : public IBinaryReadable
	{
	public:
		ObjSet() = default;
		ObjSet(ObjSet& other) = delete;
		ObjSet& operator= (ObjSet& other) = delete;
		~ObjSet() = default;

	public:
		String Name;

		ObjIterator begin() { return objects.begin(); }
		ObjIterator end() { return objects.end(); }
		ConstObjIterator begin() const { return objects.begin(); }
		ConstObjIterator end() const { return objects.end(); }
		ConstObjIterator cbegin() const { return objects.cbegin(); }
		ConstObjIterator cend() const { return objects.cend(); }

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
		virtual void Read(BinaryReader& reader) override;
		void UploadAll();

	private:
		ObjCollection objects;
	};
}