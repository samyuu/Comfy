#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "FArc.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::IO
{
	class FArcPacker : NonCopyable
	{
	public:
		FArcPacker();
		~FArcPacker();

		// NOTE: All input object references and pointers are expected to live at least until CreateFlushFArc() has been called
	public:
		void AddFile(std::string_view fileName, IStreamWritable& writable);
		void AddFile(std::string_view fileName, const void* fileContent, size_t fileSize);

	public:
		void CreateFlushFArc(std::string_view filePath);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
