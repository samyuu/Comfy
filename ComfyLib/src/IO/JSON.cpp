#include "JSON.h"
#include "File.h"

namespace Comfy
{
	std::string JSONToString(const json& json)
	{
		return json.dump(JSONDetail::Indentation, JSONDetail::IndentationChar, JSONDetail::EnsureASCII, JSONDetail::ErrorHandler);
	}

	json JSONFromString(std::string_view jsonString)
	{
		return json::parse(jsonString, nullptr, JSONDetail::AllowExceptions, JSONDetail::IgnoreComments);
	}
}

namespace Comfy::IO
{
	std::optional<json> LoadJSON(std::string_view filePath)
	{
		std::string fileContent = File::ReadAllText(filePath);
		if (fileContent.empty())
			return std::nullopt;

		return JSONFromString(fileContent);
	}

	std::future<std::optional<json>> LoadJSONAsync(std::string_view filePath)
	{
		return std::async(std::launch::async, [path = std::string(filePath)]()
		{
			return LoadJSON(path);
		});
	}

	bool SaveJSON(std::string_view filePath, const json& json)
	{
		return File::WriteAllText(filePath, JSONToString(json));
	}

	std::future<bool> SaveJSONAsync(std::string_view filePath, const json* json)
	{
		return std::async(std::launch::async, [path = std::string(filePath), json]()
		{
			return (json != nullptr) ? SaveJSON(path, *json) : false;
		});
	}
}
