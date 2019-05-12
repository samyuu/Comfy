#pragma once
#include "IEditorComponent.h"
#include "../Audio/DummySampleProvider.h"
#include "../Audio/MemoryAudioStream.h"
#include "../Audio/AudioInstance.h"
#include <vector>
#include <memory>
#include <string>

class Application;

namespace Editor
{
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
			std::string SongFileName;
		};
		// --------------

		// Song Audio Members:
		// -------------------
		struct
		{
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
		// -------------

		// Playback Control:
		// -----------------
		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();
		// -----------------
	};
}