#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Stream/FileStream.h"
#include "Stream/MemoryStream.h"
#include "Stream/Manipulator/StreamReader.h"
#include "Stream/Manipulator/StreamWriter.h"
#include "Stream/FileInterfaces.h"

namespace Comfy::IO
{
	namespace File
	{
		COMFY_NODISCARD bool Exists(std::string_view filePath);

		// NOTE: Use for mostly temporary variable, hence return by value
		COMFY_NODISCARD FileStream OpenRead(std::string_view filePath);
		COMFY_NODISCARD MemoryStream OpenReadMemory(std::string_view filePath);
		COMFY_NODISCARD FileStream CreateWrite(std::string_view filePath);

		std::pair<UniquePtr<u8[]>, size_t> ReadAllBytes(std::string_view filePath);
		bool ReadAllBytes(std::string_view filePath, std::vector<u8>& outFileContent);

		COMFY_NODISCARD std::string ReadAllText(std::string_view filePath);

		bool WriteAllBytes(std::string_view filePath, const void* dataToWrite, size_t dataSize);
		bool WriteAllText(std::string_view filePath, std::string_view text);

		template <typename Readable>
		UniquePtr<Readable> Load(std::string_view filePath)
		{
			static_assert(std::is_base_of_v<IStreamReadable, Readable>);

			auto stream = OpenReadMemory(filePath);
			if (!stream.IsOpen() || !stream.CanRead())
				return nullptr;

			auto result = MakeUnique<Readable>();
			if (result == nullptr)
				return nullptr;

			auto reader = StreamReader(stream);
			result->Read(reader);
			return result;
		}

		template <typename Writable>
		bool Save(std::string_view filePath, Writable& writable)
		{
			static_assert(std::is_base_of_v<IStreamWritable, Writable>);

			auto stream = CreateWrite(filePath);
			if (!stream.IsOpen() || !stream.CanWrite())
				return false;

			auto writer = StreamWriter(stream);
			writable.Write(writer);
			return true;
		}

	}
}
