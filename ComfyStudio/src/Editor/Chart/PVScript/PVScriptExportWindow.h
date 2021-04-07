#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Common/SoundEffectManager.h"
#include "Script/PVScript.h"
#include "PVScriptUtil.h"

namespace Comfy::Studio::Editor
{
	enum class PVScriptExportFormat : u8
	{
		Arcade,
		Orbis,
		NX,
		Count
	};

	constexpr std::array<const char*, EnumCount<PVScriptExportFormat>()> PVScriptExportFormatNames =
	{
		"Arcade",
		"PS4 (Unsupported)",
		"Switch (Unsupported)",
	};

	constexpr bool IsPVScriptExportFormatSupported(PVScriptExportFormat format)
	{
		return (format == PVScriptExportFormat::Arcade);
	};

	struct PVScriptExportWindowInputData
	{
		const Chart* Chart;
		const SoundEffectManager* SoundEffectManager;
	};

	struct PVScriptMDataExportParam
	{
		PVScriptExportFormat OutFormat;
		i32 OutPVID;
		std::string RootDirectory;
		std::array<char, 5> OutMDataID;

		vec4 PVScriptBackgroundTint;
		bool MergeWithExistingMData;
		bool CreateSprSelPV;
		bool AddDummyMovieReference;

		std::string MDataRootDirectory;

		std::string InMergeBasePVListFArc;
		std::string InMergeBaseMDataPVDB;
		std::string InMergeBaseMDataSprDB;

		std::string OutMDataDirectory;
		std::string OutMDataRomDirectory;
		std::string OutMDataRom2DDirectory;
		std::string OutMDataInfo;
		std::string OutOgg;
		std::string OutDsc;
		std::string OutPVListFArc;
		std::string OutMDataPVDB;
		std::string OutMDataSprDB;
		std::string OutSprSelPVFArc;
	};

	class PVScriptExportWindow : NonCopyable
	{
	public:
		PVScriptExportWindow();
		~PVScriptExportWindow() = default;

	public:
		void Gui(const PVScriptExportWindowInputData& inData);
		bool GetAndClearCloseRequestThisFrame();

		void OnWindowOpen();
		void OnCloseButtonClicked();

		void ConvertAndSaveSimpleScriptSync(std::string_view outputScriptPath, const Chart& chart) const;

	private:
		void StartExport(const PVScriptExportWindowInputData& inData);
		void RequestExit();

		void InternalOnOpen();
		void InternalOnClose();

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;

		PVScriptMDataExportParam param = {};
	};
}
