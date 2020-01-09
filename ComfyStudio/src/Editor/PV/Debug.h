#pragma once
#include "StageTest.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Debug
{
	enum class PathType
	{
		StageObj,
		StageTxp,
		CharaItemObj,
		CharaItemTxp,
		Fog,
		Glow,
		Light,
		IBL,
	};

	std::array<char, MAX_PATH> GetDebugFilePath(PathType type, Editor::StageType stageType, int id, int subID = 0, const char* chara = nullptr)
	{
		constexpr std::array fileNameFormatStrings = { "tst%03d", "ns%03d", "d2ns%03d", "pv%03ds%02d" };
		const char* fileNameFormatString = fileNameFormatStrings[static_cast<size_t>(stageType)];

		if (id == 0 && stageType == Editor::StageType::STGTST)
			fileNameFormatString = "tst";
		else if (subID == 0 && stageType == Editor::StageType::STGPV)
			fileNameFormatString = "pv%03d";

		std::array<char, MAX_PATH> path = {};

		std::array<char, 16> fileName = {};
		sprintf_s(fileName.data(), fileName.size(), fileNameFormatString, id, subID);

		auto formatFileNameOrDefault = [&](const char* formatPath, const char* defaultPath)
		{
			sprintf_s(path.data(), path.size(), formatPath, fileName.data());

			if (!FileSystem::FileExists(path.data()))
				strcpy_s(path.data(), path.size(), defaultPath);
		};

		const char* fileNameSuffix = (type == PathType::StageObj || type == PathType::CharaItemObj) ? "obj" : "tex";
		if (type == PathType::StageObj || type == PathType::StageTxp)
		{
			constexpr std::array stgTypeStrings = { "stgtst", "stgns", "stgd2ns", "stgpv" };
			const char* stgTypeString = stgTypeStrings[static_cast<size_t>(stageType)];

			sprintf_s(path.data(), path.size(), "dev_rom/objset/stg/%s/stg%s/stg%s_%s.bin", stgTypeString, fileName.data(), fileName.data(), fileNameSuffix);
		}
		else if (type == PathType::CharaItemObj || type == PathType::CharaItemTxp)
		{
			sprintf_s(path.data(), path.size(), "dev_rom/objset/chritm/%sitm/%sitm%03d/%sitm%03d_%s.bin", chara, chara, id, chara, id, fileNameSuffix);
		}
		else if (type == PathType::Fog)
			formatFileNameOrDefault("dev_rom/light_param/fog_%s.txt", "dev_rom/light_param/fog_tst.txt");
		else if (type == PathType::Glow)
			formatFileNameOrDefault("dev_rom/light_param/glow_%s.txt", "dev_rom/light_param/glow_tst.txt");
		else if (type == PathType::Light)
			formatFileNameOrDefault("dev_rom/light_param/light_%s.txt", "dev_rom/light_param/light_tst.txt");
		else if (type == PathType::IBL)
			formatFileNameOrDefault("dev_rom/ibl/%s.ibl", "dev_rom/ibl/tst.ibl");

		return path;
	}

	template <typename T>
	bool LoadParseUploadLightParamFile(std::string_view filePath, T& param)
	{
		if (!FileSystem::FileExists(filePath))
			return false;

		std::vector<uint8_t> fileContent;
		FileSystem::FileReader::ReadEntireFile(filePath, &fileContent);

		param.Parse(fileContent.data(), fileContent.size());

		if constexpr (std::is_same<T, Graphics::LightDataIBL>::value)
			param.UploadAll();

		return true;
	}

	void LoadStageLightParamFiles(Graphics::SceneContext& context, Editor::StageType stageType, int stageID, int stageSubID = 0)
	{
		auto pathBuffer = GetDebugFilePath(PathType::Fog, stageType, stageID, stageSubID);
		LoadParseUploadLightParamFile(pathBuffer.data(), context.Fog);
		
		pathBuffer = GetDebugFilePath(PathType::Glow, stageType, stageID, stageSubID);
		LoadParseUploadLightParamFile(pathBuffer.data(), context.Glow);
		
		pathBuffer = GetDebugFilePath(PathType::Light, stageType, stageID, stageSubID);
		LoadParseUploadLightParamFile(pathBuffer.data(), context.Light);
		
		pathBuffer = GetDebugFilePath(PathType::IBL, stageType, stageID, stageSubID);
		LoadParseUploadLightParamFile(pathBuffer.data(), context.IBL);
	}

	std::string GetTxpSetPathForObjSet(std::string_view objSetPath)
	{
		std::string txpSetPath;
		txpSetPath.reserve(MAX_PATH);

		std::string_view fileName = FileSystem::GetFileName(objSetPath, false);
		std::string_view objName = fileName.substr(0, fileName.length() - strlen("_obj"));

		txpSetPath.append(FileSystem::GetDirectory(objSetPath));
		txpSetPath.append("/");
		txpSetPath.append(objName);
		txpSetPath.append("_tex.bin");

		return txpSetPath;
	}

	bool IsGroundObj(const Graphics::Obj& obj)
	{
		return EndsWithInsensitive(obj.Name, "_gnd");
	}

	bool IsReflectionObj(const Graphics::Obj& obj)
	{
		return EndsWithInsensitive(obj.Name, "_reflect");
	}

	int FindGroundObj(Graphics::ObjSet* objSet)
	{
		if (objSet == nullptr)
			return -1;

		auto result = std::find_if(objSet->begin(), objSet->end(), [](auto& obj) { return IsGroundObj(obj); });
		return (result == objSet->end()) ? -1 : static_cast<int>(std::distance(objSet->begin(), result));
	}
}
