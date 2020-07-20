#include "ObjSet.h"
#include "IO/File.h"

namespace Comfy::Graphics
{
	IndexFormat SubMesh::GetIndexFormat() const
	{
		return static_cast<IndexFormat>(Indices.index());
	}

	std::vector<u8>* SubMesh::GetIndicesU8()
	{
		return std::get_if<std::vector<u8>>(&Indices);
	}

	const std::vector<u8>* SubMesh::GetIndicesU8() const
	{
		return std::get_if<std::vector<u8>>(&Indices);
	}

	std::vector<u16>* SubMesh::GetIndicesU16()
	{
		return std::get_if<std::vector<u16>>(&Indices);
	}

	const std::vector<u16>* SubMesh::GetIndicesU16() const
	{
		return std::get_if<std::vector<u16>>(&Indices);
	}

	std::vector<u32>* SubMesh::GetIndicesU32()
	{
		return std::get_if<std::vector<u32>>(&Indices);
	}

	const std::vector<u32>* SubMesh::GetIndicesU32() const
	{
		return std::get_if<std::vector<u32>>(&Indices);
	}

	const size_t SubMesh::GetIndexCount() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return indices->size();

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return indices->size();

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return indices->size();

		return 0;
	}

	const void* SubMesh::GetRawIndices() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		return nullptr;
	}

	size_t SubMesh::GetRawIndicesByteSize() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return indices->size() * sizeof(u8);

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return indices->size() * sizeof(u16);

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return indices->size() * sizeof(u32);

		return 0;
	}
}
