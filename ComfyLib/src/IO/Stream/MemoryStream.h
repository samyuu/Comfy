#pragma once
#include "Types.h"
#include "IStream.h"

namespace Comfy::IO
{
	// TODO: Update and correctly handle writing
	class MemoryStream final : public IStream, NonCopyable
	{
	public:
		MemoryStream();
		MemoryStream(MemoryStream&& other);
		~MemoryStream();

	public:
		void Seek(FileAddr position) override;
		FileAddr GetPosition() const override;
		FileAddr GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;
		bool IsOwning() const;

		size_t ReadBuffer(void* buffer, size_t size) override;
		size_t WriteBuffer(const void* buffer, size_t size) override;

		void FromStreamSource(std::vector<u8>& source);
		void FromStream(IStream& stream);
		
		template <typename Func>
		void FromBuffer(size_t size, Func fromBufferFunc)
		{
			isOpen = true;
			owningDataVector.resize(size);
			fromBufferFunc(static_cast<void*>(owningDataVector.data()), size);
			dataSize = static_cast<FileAddr>(size);
		}

		void Close() override;

	protected:
		bool isOpen = false;

		FileAddr position = {};
		FileAddr dataSize = {};

		// TODO: Refactor this, consider using u8[] unique ptr instead
		std::vector<u8>* dataVectorPtr = nullptr;
		std::vector<u8> owningDataVector;
	};
}
