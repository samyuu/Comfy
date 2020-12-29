#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	class RecentFilesList
	{
	public:
		RecentFilesList(size_t maxCount = 9) : maxFileCount(maxCount) {}
		~RecentFilesList() = default;

	public:
		void FromNewLineString(std::string_view newLineList);
		std::string_view ToNewLineString();

		const std::vector<std::string>& View() const;

		void Add(std::string_view filePath);
		void RemoveAt(size_t index);

		void Clear();

	private:
		size_t maxFileCount;
		std::vector<std::string> filePaths;

		std::string newLineBuffer;
	};
}
