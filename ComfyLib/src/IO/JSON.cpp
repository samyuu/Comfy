#include "JSON.h"
#include "File.h"

namespace Comfy
{
	std::string JsonToString(const json& json)
	{
		return json.dump(JsonDetail::Indentation, JsonDetail::IndentationChar, JsonDetail::EnsureASCII, JsonDetail::ErrorHandler);
	}

	json JsonFromString(std::string_view jsonString)
	{
		return json::parse(jsonString, nullptr, JsonDetail::AllowExceptions, JsonDetail::IgnoreComments);
	}
}

namespace Comfy::IO
{
	std::optional<json> LoadJson(std::string_view filePath)
	{
		std::string fileContent = File::ReadAllText(filePath);
		if (fileContent.empty())
			return std::nullopt;

		return JsonFromString(fileContent);
	}

	std::future<std::optional<json>> LoadJsonAsync(std::string_view filePath)
	{
		return std::async(std::launch::async, [path = std::string(filePath)]()
		{
			return LoadJson(path);
		});
	}

	bool SaveJson(std::string_view filePath, const json& json)
	{
		return File::WriteAllText(filePath, JsonToString(json));
	}

	std::future<bool> SaveJsonAsync(std::string_view filePath, const json* json)
	{
		return std::async(std::launch::async, [path = std::string(filePath), json]()
		{
			return (json != nullptr) ? SaveJson(path, *json) : false;
		});
	}
}
