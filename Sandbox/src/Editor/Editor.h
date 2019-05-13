#pragma once
#include "IEditorComponent.h"
#include "../Audio/DummySampleProvider.h"
#include "../Audio/MemoryAudioStream.h"
#include "../Audio/AudioInstance.h"
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

class Application;

namespace Editor
{
	namespace FileSystem = std::filesystem;

	class PvEditor
	{
		friend class TargetTimeline;

	public:
		// Constructors / Destructors:
		// ---------------------------
		PvEditor(Application* parent);
		~PvEditor();
		// ---------------------------

		// Application Methods:
		// --------------------
		void DrawGuiMenuItems();
		void DrawGuiWindows();
		// --------------------

		// PV Control:
		// -----------
		bool Load(const std::string& filePath);
		bool LoadSong(const std::string& filePath);
		// -----------

	private:
		
		// Base Members:
		// -------------
		Application* parent;

		std::vector<std::unique_ptr<IEditorComponent>> editorComponents;
		bool initialized = false;
		// -------------

		// PV Properties:
		// --------------
		struct
		{
			std::string SongName;
			std::string SongNameReading;
			FileSystem::path SongFileName;
		};
		// --------------

		// Song Audio Members:
		// -------------------
		struct
		{
			const char* audioFileExtensions[4] = { ".wav", ".flac", ".ogg", ".mp3" };

			DummySampleProvider dummySampleProvider;

			std::shared_ptr<MemoryAudioStream> songStream;
			std::shared_ptr<AudioInstance> songInstance;
		};
		// -------------------

		// Playback Members:
		// -----------------
		struct
		{
			bool isPlaying = false;
			
			TimeSpan songStartOffset = 0.0;
			TimeSpan songDuration = TimeSpan::FromMinutes(1.0);
			
			TimeSpan playbackTime = 0.0;
			TimeSpan playbackTimeOnPlaybackStart;
		};
		// -----------------

		// Base Methods:
		// -------------
		void Initialize();
		void Update();
		void DrawGui();
		void UpdateFileDrop();
		void UpdatePlayback();
		// -------------

		// Playback Control:
		// -----------------
		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();
		// -----------------
	};
}