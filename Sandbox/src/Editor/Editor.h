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
		PvEditor(Application* parent);
		~PvEditor();

		void DrawGuiMenuItems();
		void DrawGuiWindows();

	private:
		Application* parent;
		std::vector<std::unique_ptr<IEditorComponent>> editorComponents;
		bool initialized = false;

		struct
		{
			std::string SongName;
			std::string SongNameReading;
			std::string SongFileName;
		};

		struct
		{
			DummySampleProvider dummySampleProvider;
			std::shared_ptr<MemoryAudioStream> songStream;
			std::shared_ptr<AudioInstance> songInstance;
		};

		struct
		{
			TimeSpan songStartOffset = 0.0;
			TimeSpan songDuration = TimeSpan::FromMinutes(1.0);
			TimeSpan playbackTime = 0.0, playbackTimeOnPlaybackStart;
			bool isPlaying = false;
		};

		// Base Methods
		// ------------
		void Initialize();
		// ------------

		// Playback Control:
		// -----------------
		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();
		// -----------------
	};
}