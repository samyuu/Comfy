#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Misc/ImageHelper.h"
#include "Graphics/Utilities/SpritePacker.h"

namespace Comfy::Studio::Editor
{
	class DanceBuddyWindow
	{
	public:
		DanceBuddyWindow()
		{
		}

		DanceBuddyWindow(const DanceBuddyWindow& other)
		{
			pathBuffer = other.pathBuffer;
			beatsPerLoop = other.beatsPerLoop;
			fillWindow = other.fillWindow;
			sprSet = other.sprSet;
		}

		~DanceBuddyWindow()
		{
			if (loadFuture.valid())
				sprSet = loadFuture.get();
		}

	public:
		void BeginEndGui(const TargetTimeline& timeline)
		{
			const auto cursorTick = timeline.GetCursorTick();
			const f32 cursorProgress = glm::mod(cursorTick.BeatsFraction(), beatsPerLoop);

			const auto imageCount = (sprSet == nullptr) ? 0 : sprSet->Sprites.size();

			const f32 cursorFrame = (static_cast<f32>(imageCount) / static_cast<f32>(beatsPerLoop)) * cursorProgress;
			const i32 frameIndex = std::clamp(static_cast<i32>(cursorFrame), 0, static_cast<i32>(imageCount));

			Gui::PushID(this);
			char windowTitle[128];
			sprintf_s(windowTitle, "Dance Buddy##%p", this);

			duplicationRequested = false;

			if (loadFuture._Is_ready())
				sprSet = loadFuture.get();

			if (Gui::Begin(windowTitle, &isOpen, ImGuiWindowFlags_NoSavedSettings))
			{
				Gui::WindowContextMenu("DanceBuddyWindowContextMenu", [&]()
				{
					if (Gui::InputText("Path Format", pathBuffer.data(), pathBuffer.size(), 0))
					{
						constexpr auto patternSuffix = std::string_view("_000.png");

						auto path = std::string(IO::Path::TrimQuotes(pathBuffer.data()));

						if (IO::Path::IsDirectory(path))
							path = IO::Path::Combine(path, std::string(IO::Path::GetFileName(path)) + std::string(patternSuffix));

						if (Util::EndsWith(path, patternSuffix))
							path = std::string(Util::StripSuffix(path, patternSuffix)) + "_%03d.png";

						memcpy(pathBuffer.data(), path.data(), std::clamp(path.size() + 1, static_cast<size_t>(0), pathBuffer.size() - 1));
						LoadImages(pathBuffer.data(), -1);
					}

					Gui::Text("Cursor Frame: %.3f", cursorFrame);
					Gui::DragFloat("Beats Per Loop", &beatsPerLoop, 0.01f);

					Gui::Checkbox("Fill Winodw", &fillWindow);

					if (Gui::Button("Duplicate"))
						duplicationRequested = true;
				});

				if (sprSet != nullptr && InBounds(frameIndex, sprSet->Sprites))
				{
					const auto& spr = sprSet->Sprites[frameIndex];
					const auto& tex = *sprSet->TexSet.Textures[spr.TextureIndex];

					const vec2 uv0 = vec2(spr.TexelRegion.x, spr.TexelRegion.y);
					const vec2 uv1 = uv0 + vec2(spr.TexelRegion.z, spr.TexelRegion.w);

					if (fillWindow)
					{
						const auto imageRegion = ImRect(Gui::GetCursorScreenPos(), Gui::GetCursorScreenPos() + Gui::GetContentRegionAvail());
						const auto adjustedRegion = Gui::FitFixedAspectRatioImage(imageRegion, spr.GetSize());

						Gui::SetCursorScreenPos(adjustedRegion.GetTL());
						Gui::Image(tex, adjustedRegion.GetSize(), uv0, uv1);
					}
					else
					{
						Gui::Image(tex, spr.GetSize(), uv0, uv1);
					}
				}
			}
			Gui::End();

			Gui::PopID();
		}

		bool IsOpen() const
		{
			return isOpen;
		}

		bool DuplicatedRequested() const
		{
			return duplicationRequested;
		}

		void LoadImages(const char* formatString, i32 frameCount = -1)
		{
			if (lastLoadedFormatString == formatString)
				return;
			lastLoadedFormatString = formatString;

			if (loadFuture.valid())
				sprSet = loadFuture.get();

			sprSet = std::make_shared<Graphics::SprSet>();

			if (formatString == nullptr)
				return;

			if (frameCount < 0)
				frameCount = 512;

			loadFuture = std::async(std::launch::async, [formatString, frameCount]
			{
				struct ImageFile
				{
					ImageFile(std::string_view filePath) { Util::ReadImage(filePath, Size, Pixels); }

					ivec2 Size;
					std::unique_ptr<u8[]> Pixels;
				};

				Graphics::Utilities::SpritePacker packer = {};
				packer.Settings.FlipTexturesY = false;

				std::vector<ImageFile> sourceImages;
				sourceImages.reserve(frameCount);

				std::vector<Graphics::Utilities::SprMarkup> sprMarkups;
				sprMarkups.reserve(frameCount);

				for (i32 i = 0; i < frameCount; i++)
				{
					char imagePath[260];
					sprintf_s(imagePath, formatString, i);

					auto& sourceImage = sourceImages.emplace_back(imagePath);
					if (sourceImage.Pixels == nullptr)
						break;

					char indexStr[12];
					sprintf_s(indexStr, "%08d", i);

					auto& markup = sprMarkups.emplace_back();
					markup.Name = indexStr;
					markup.Size = sourceImage.Size;
					markup.RGBAPixels = sourceImage.Pixels.get();
					markup.ScreenMode = Graphics::ScreenMode::HDTV1080;
					markup.Flags = Graphics::Utilities::SprMarkupFlags_Compress;
				}

				return packer.Create(sprMarkups);
			});
		}

	private:
		std::string lastLoadedFormatString;

		std::array<char, 260> pathBuffer = { "Y:/Nop/Emotes/FrameSplitGIFs/dummy_%03d.png" };
		f32 beatsPerLoop = 4.0f;

		bool fillWindow = true;

		bool isOpen = true;
		bool duplicationRequested = false;

		std::future<std::unique_ptr<Graphics::SprSet>> loadFuture;
		std::shared_ptr<Graphics::SprSet> sprSet;
	};

	class DanceBuddyWindowManager
	{
	public:
		DanceBuddyWindowManager()
		{
			danceBuddyWindows.emplace_back(std::make_unique<DanceBuddyWindow>());
		}

	public:
		void DrawGui(const TargetTimeline& timeline)
		{
			for (auto& window : danceBuddyWindows)
			{
				window->BeginEndGui(timeline);
			}

			for (auto& window : danceBuddyWindows)
			{
				if (window->DuplicatedRequested())
				{
					danceBuddyWindows.emplace_back(std::make_unique<DanceBuddyWindow>(*window));
					break;
				}
				else if (!window->IsOpen())
				{
					danceBuddyWindows.erase(danceBuddyWindows.begin() + std::distance(&danceBuddyWindows.front(), &window));
					break;
				}
			}
		}

	private:
		std::vector<std::unique_ptr<DanceBuddyWindow>> danceBuddyWindows;
	};
}
