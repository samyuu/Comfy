#pragma once
#include "Types.h"
#include "Core/BaseWindow.h"
#include "Render/Movie/MoviePlayer.h"

namespace Comfy::Studio::DataTest
{
	class MovieTestWindow : public BaseWindow
	{
	public:
		MovieTestWindow(ComfyStudioApplication&);
		~MovieTestWindow() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	private:
		struct MovieViewData
		{
			std::unique_ptr<Render::IMoviePlayer> Player = nullptr;
			std::string FilePathBuffer;
			bool LoadFileFullyIntoMemory = false;
			bool ReloadNextFrame = false;
			bool RemoveNextFrame = false;
		};

		void MovieViewGui(MovieViewData& movieView);
		
	private:
		std::vector<MovieViewData> movieViews;
		bool isFirstFrame = true;
	};
}
