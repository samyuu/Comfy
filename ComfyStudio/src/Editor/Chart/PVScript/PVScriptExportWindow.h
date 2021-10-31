#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "ImGui/Widgets/LoadingTextAnimation.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Common/SoundEffectManager.h"
#include "Script/PVScript.h"
#include "PVScriptUtil.h"

namespace Comfy::Studio::Editor
{
	enum class PVExportFormat : u8
	{
		Arcade,
		Orbis,
		NX,
		Count
	};

	constexpr std::array<const char*, EnumCount<PVExportFormat>()> PVExportFormatNames =
	{
		"Arcade",
		"PS4 (Unsupported)",
		"Switch (Unsupported)",
	};

	constexpr bool IsPVExportFormatSupported(PVExportFormat format)
	{
		return (format == PVExportFormat::Arcade);
	};

	struct PVExportWindowInputData
	{
		const Chart* Chart;
		const SoundEffectManager* SoundEffectManager;
		std::shared_ptr<Audio::ISampleProvider> SongSampleProvider;
	};

	struct PVMDataExportParam
	{
		PVExportFormat OutFormat;
		i32 OutPVID;
		std::string RootDirectory;
		std::array<char, 5> OutMDataID;

		vec4 PVScriptBackgroundTint;
		bool MergeWithExistingMData;
		bool CreateSprSelPV;
		bool AddDummyMovieReference;
		f32 VorbisVBRQuality;

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

		std::shared_ptr<Audio::ISampleProvider> SongSampleProvider;
	};

	struct PVExportAtomicProgress
	{
		std::atomic<f32> Audio;
		std::atomic<f32> Sprites;
		std::atomic<f32> Script;
		std::atomic<f32> MDataInfo;
		std::atomic<f32> PVDB;
		std::atomic<f32> PVList;

		PVExportAtomicProgress() = default;
		~PVExportAtomicProgress() = default;

		inline void Reset()
		{
			// HACK: Is this well defined behavior?
			this->~PVExportAtomicProgress();
			new (this) PVExportAtomicProgress();
		}
	};

	using PVExportTasks = std::vector<std::future<void>>;

	class PVScriptExportWindow : NonCopyable
	{
	public:
		PVScriptExportWindow();
		~PVScriptExportWindow();

	public:
		void Gui(const PVExportWindowInputData& inData);
		bool GetAndClearCloseRequestThisFrame();

		void OnWindowOpen();
		void OnCloseButtonClicked();

		void ConvertAndSaveSimpleScriptSync(std::string_view outputScriptPath, const Chart& chart) const;

	private:
		void StartAsyncExport(PVExportWindowInputData inData);
		void RequestExit();

		void InternalOnOpen();
		void InternalOnClose();

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;

		bool isCurrentlyAsyncExporting = false;

		PVMDataExportParam param = {};
		PVExportTasks tasks = {};
		PVExportAtomicProgress progress = {};
		PVExportWindowInputData asyncAccessedWindowInputData = {};

		Gui::BinaryLoadingTextAnimation loadingAnimation = {};
	};
}
