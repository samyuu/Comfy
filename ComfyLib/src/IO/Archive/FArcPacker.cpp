#include "FArcPacker.h"
#include "IO/File.h"
#include "IO/Stream/FileStream.h"
#include "IO/Stream/MemoryWriteStream.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::IO
{
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
		};

		struct DataPointerEntry : EntryBase
		{
			DataPointerEntry(std::string_view fileName, const void* data, size_t dataSize) : EntryBase(fileName), Data(data), DataSize(dataSize) {}

			const void* Data;
			size_t DataSize;
		};

		std::vector<StreamWritableEntry> WritableEntries;
		std::vector<DataPointerEntry> DataPointerEntries;

		void CreateFArc(std::string_view filePath, u32 alignment = 16, bool compressed = false)
		{
			assert(!compressed);

			auto outputFileStream = File::CreateWrite(filePath);
			auto farcWriter = StreamWriter(outputFileStream);
			farcWriter.SetEndianness(Endianness::Big);
			farcWriter.SetPointerMode(PtrMode::Mode32Bit);

			farcWriter.WriteU32(static_cast<u32>(FArcSignature::UnCompressed));
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

					farcWriter.WriteBuffer(fileDataBuffer.get(), entry.FileSizeOnceWritten);
					farcWriter.WriteAlignmentPadding(alignment);
				});
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
					farcWriter.WriteBuffer(entry.Data, entry.DataSize);
					farcWriter.WriteAlignmentPadding(alignment);
				});
				farcWriter.WriteSize(entry.DataSize);
			}

			delayedHeaderSize = static_cast<u32>(farcWriter.GetPosition()) - (sizeof(u32) * 2);
			farcWriter.WriteAlignmentPadding(alignment);

			farcWriter.FlushPointerPool();
			farcWriter.FlushDelayedWritePool();
			farcWriter.WriteAlignmentPadding(alignment);
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

	void FArcPacker::CreateFlushFArc(std::string_view filePath)
	{
		impl->CreateFArc(filePath);
		impl->ClearAll();
	}
}
