#include "RecentFilesList.h"
#include "Misc/StringParseHelper.h"
#include <numeric>

namespace Comfy::Studio::Editor
{
	void RecentFilesList::FromNewLineString(std::string_view newLineList)
	{
		filePaths.clear();
		if (filePaths.capacity() < maxFileCount)
			filePaths.reserve(maxFileCount);

		if (newLineList.empty())
			return;

		const char* readHead = newLineList.data();
		const char* end = newLineList.data() + newLineList.size();

		while (readHead < end)
		{
			const auto pathLine = Util::StringParsing::GetLine(readHead);
			Util::StringParsing::AdvanceToNextLine(readHead);

			filePaths.emplace_back(pathLine);
		}
	}

	std::string_view RecentFilesList::ToNewLineString()
	{
		const auto requiredCharacters = std::accumulate(filePaths.begin(), filePaths.end(), static_cast<size_t>(0),
			[](const auto count, const auto& path) { return count + (path.size() + sizeof('\n')); });

		newLineBuffer.clear();
		newLineBuffer.reserve(requiredCharacters);

		if (filePaths.size() < 1)
			return newLineBuffer;

		for (size_t i = 0; i < filePaths.size() - 1; i++)
		{
			newLineBuffer += filePaths[i];
			newLineBuffer += '\n';
		}
		newLineBuffer += filePaths.back();

		return newLineBuffer;
	}

	const std::vector<std::string>& RecentFilesList::View() const
	{
		return filePaths;
	}

	void RecentFilesList::Add(std::string_view filePath)
	{
		if (maxFileCount < 1)
			return;

		if (filePaths.capacity() < maxFileCount)
			filePaths.reserve(maxFileCount);

		// TODO: Normalize and make absolute (?)
		const auto existingIndex = FindIndexOf(filePaths, [&](const auto& p) { return p == filePath; });
		if (InBounds(existingIndex, filePaths))
		{
			auto existing = std::move(filePaths[existingIndex]);
			filePaths.erase(filePaths.begin() + existingIndex);
			filePaths.push_back(std::move(existing));
		}
		else
		{
			if (!filePaths.empty() && filePaths.size() + 1 > maxFileCount)
				filePaths.erase(filePaths.begin());

			filePaths.push_back(std::string(filePath));
		}
	}

	void RecentFilesList::RemoveAt(size_t index)
	{
		assert(index < filePaths.size());
		filePaths.erase(filePaths.begin() + index);
	}

	void RecentFilesList::Clear()
	{
		filePaths.clear();
	}
}
