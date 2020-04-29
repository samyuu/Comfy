#pragma once
#include "IStream.h"
#include "CoreTypes.h"

namespace Comfy::IO
{
	class MemoryStream final : public IStream, NonCopyable
	{
	public:
		MemoryStream();
		explicit MemoryStream(std::string_view filePath);
		explicit MemoryStream(IStream& stream);
		explicit MemoryStream(std::vector<u8>& source);
		~MemoryStream();

		void Seek(FileAddr position) override;
		FileAddr GetPosition() const override;
		FileAddr GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;

		size_t ReadBuffer(void* buffer, size_t size) override;
		size_t WriteBuffer(const void* buffer, size_t size) override;

		void FromStreamSource(std::vector<u8>& source);
		void FromFile(std::string_view filePath);
		void FromStream(IStream& stream);
		void Close() override;

	protected:
		bool canRead = false;

		FileAddr position = {};
		FileAddr dataSize = {};

		std::vector<u8>* dataSource;
		std::vector<u8> dataVector;
	};
}
