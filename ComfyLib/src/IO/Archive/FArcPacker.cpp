#include "FArcPacker.h"
#include "IO/File.h"
#include "IO/Stream/FileStream.h"
#include "IO/Stream/MemoryWriteStream.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include <zlib.h>

namespace Comfy::IO
{
	namespace
	{
		size_t CompressBufferIntoStream(const void* inData, size_t inDataSize, StreamWriter& outWriter)
		{
			constexpr size_t chunkStepSize = 0x4000;

			z_stream zStream = {};
			zStream.zalloc = Z_NULL;
			zStream.zfree = Z_NULL;
			zStream.opaque = Z_NULL;

			int errorCode = deflateInit2(&zStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
			assert(errorCode == Z_OK);

			const u8* inDataReadHeader = static_cast<const u8*>(inData);
			size_t remainingSize = inDataSize;
			size_t compressedSize = 0;

			while (remainingSize > 0)
			{
				const size_t chunkSize = std::min(remainingSize, chunkStepSize);

				zStream.avail_in = static_cast<uInt>(chunkSize);
				zStream.next_in = reinterpret_cast<const Bytef*>(inDataReadHeader);

				inDataReadHeader += chunkSize;
				remainingSize -= chunkSize;

				do
				{
					std::array<u8, chunkStepSize> outputBuffer;

					zStream.avail_out = chunkStepSize;
					zStream.next_out = outputBuffer.data();

					errorCode = deflate(&zStream, remainingSize == 0 ? Z_FINISH : Z_NO_FLUSH);
					assert(errorCode != Z_STREAM_ERROR);

					const auto compressedChunkSize = chunkStepSize - zStream.avail_out;
					outWriter.WriteBuffer(outputBuffer.data(), compressedChunkSize);

					compressedSize += compressedChunkSize;
				}
				while (zStream.avail_out == 0);
				assert(zStream.avail_in == 0);
			}

			deflateEnd(&zStream);

			assert(errorCode == Z_STREAM_END);
			return compressedSize;
		}
	}

	struct FArcPacker::Impl
	{
		struct EntryBase
		{
			EntryBase(std::string_view fileName) : FileName(fileName) {}

			std::string FileName;
		};

		struct StreamWritableEntry : EntryBase
		{
			StreamWritableEntry(std::string_view fileName, IStreamWritable& writable) : EntryBase(fileName), Writable(writable), FileSizeOnceWritten(0) {}

			IStreamWritable& Writable;
			size_t FileSizeOnceWritten;
			size_t CompressedFileSizeOnceWritten;
		};

		struct DataPointerEntry : EntryBase
		{
			DataPointerEntry(std::string_view fileName, const void* data, size_t dataSize) : EntryBase(fileName), Data(data), DataSize(dataSize) {}

			const void* Data;
			size_t DataSize;
			size_t CompressedFileSizeOnceWritten;
		};

		std::vector<StreamWritableEntry> WritableEntries;
		std::vector<DataPointerEntry> DataPointerEntries;

		bool CreateFArc(std::string_view filePath, bool compressed, u32 alignment = 16)
		{
			auto outputFileStream = File::CreateWrite(filePath);

			if (!outputFileStream.IsOpen())
				return false;

			auto farcWriter = StreamWriter(outputFileStream);
			farcWriter.SetEndianness(Endianness::Big);
			farcWriter.SetPointerMode(PtrMode::Mode32Bit);

			farcWriter.WriteU32(static_cast<u32>(compressed ? FArcSignature::Compressed : FArcSignature::UnCompressed));
			u32 delayedHeaderSize = 0;
			farcWriter.WriteDelayedPtr([&delayedHeaderSize](StreamWriter& writer) {writer.WriteU32(delayedHeaderSize); });
			farcWriter.WriteU32(alignment);

			for (auto& entry : WritableEntries)
			{
				farcWriter.WriteStr(entry.FileName);
				farcWriter.WriteFuncPtr([&](StreamWriter& writer)
				{
					std::unique_ptr<u8[]> fileDataBuffer;
					auto fileWriteMemoryStream = MemoryWriteStream(fileDataBuffer);
					auto fileWriter = StreamWriter(fileWriteMemoryStream);

					entry.Writable.Write(fileWriter);
					entry.FileSizeOnceWritten = static_cast<size_t>(fileWriteMemoryStream.GetLength());
					entry.CompressedFileSizeOnceWritten = entry.FileSizeOnceWritten;

					if (compressed)
						entry.CompressedFileSizeOnceWritten = CompressBufferIntoStream(fileDataBuffer.get(), entry.FileSizeOnceWritten, writer);
					else
						farcWriter.WriteBuffer(fileDataBuffer.get(), entry.FileSizeOnceWritten);

					farcWriter.WriteAlignmentPadding(alignment);
				});

				if (compressed)
				{
					farcWriter.WriteDelayedPtr([&](StreamWriter& writer)
					{
						writer.WriteSize(entry.CompressedFileSizeOnceWritten);
					});
				}

				farcWriter.WriteDelayedPtr([&](StreamWriter& writer)
				{
					writer.WriteSize(entry.FileSizeOnceWritten);
				});
			}

			for (auto& entry : DataPointerEntries)
			{
				farcWriter.WriteStr(entry.FileName);
				farcWriter.WriteFuncPtr([&](StreamWriter& writer)
				{
					if (compressed)
						entry.CompressedFileSizeOnceWritten = CompressBufferIntoStream(entry.Data, entry.DataSize, writer);
					else
						farcWriter.WriteBuffer(entry.Data, entry.DataSize);

					farcWriter.WriteAlignmentPadding(alignment);
				});

				if (compressed)
				{
					farcWriter.WriteDelayedPtr([&](StreamWriter& writer)
					{
						writer.WriteSize(entry.CompressedFileSizeOnceWritten);
					});
				}

				farcWriter.WriteSize(entry.DataSize);
			}

			delayedHeaderSize = static_cast<u32>(farcWriter.GetPosition()) - (sizeof(u32) * 2);
			farcWriter.WriteAlignmentPadding(alignment);

			farcWriter.FlushPointerPool();
			farcWriter.FlushDelayedWritePool();
			farcWriter.WriteAlignmentPadding(alignment);

			return true;
		}

		void ClearAll()
		{
			WritableEntries.clear();
			DataPointerEntries.clear();
		}
	};

	FArcPacker::FArcPacker() : impl(std::make_unique<Impl>())
	{
	}

	FArcPacker::~FArcPacker()
	{
		// NOTE: Not calling flush after having added files is unlikely to be intended
		assert(impl->WritableEntries.empty() && impl->DataPointerEntries.empty());
	}

	void FArcPacker::AddFile(std::string_view fileName, IStreamWritable& writable)
	{
		impl->WritableEntries.emplace_back(fileName, writable);
	}

	void FArcPacker::AddFile(std::string_view fileName, const void* fileContent, size_t fileSize)
	{
		if (fileContent != nullptr)
			impl->DataPointerEntries.emplace_back(fileName, fileContent, fileSize);
	}

	bool FArcPacker::CreateFlushFArc(std::string_view filePath, bool compressed)
	{
		const bool result = impl->CreateFArc(filePath, compressed);
		impl->ClearAll();
		return result;
	}
}
