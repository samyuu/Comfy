#include "ChartPropertiesWindow.h"
#include "ChartEditor.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

namespace Comfy::Studio::Editor
{
	ChartPropertiesWindow::ChartPropertiesWindow(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
	}

	void ChartPropertiesWindow::Gui(Chart& chart)
	{
		constexpr const char* defaultHint = "n/a";

		GuiPropertyRAII::PropertyValueColumns columns;
		GuiProperty::TreeNode("Chart Properties", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			// NOTE: Operate on properties directly without undo actions because they all refer to the same chart object and are typically only set once. 
			//		 Accidentally undoing them while placing targets for example is never desierable and text boxes already have an internal undo stack implemented

			bool changesMade = false;

			changesMade |= GuiProperty::PropertyLabelValueFunc("Song File Name", [&]
			{
				const auto& style = Gui::GetStyle();
				const auto buttonSize = Gui::GetFrameHeight();

				const bool songIsLoading = chartEditor.IsSongAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(songIsLoading);

				Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvailWidth() - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));
				if (Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, (songIsLoading ? "Loading..." : defaultHint), &chart.SongFileName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					chartEditor.LoadSongAsync(chart.SongFileName);
					changesMade = true;
				}
				Gui::PopItemWidth();

				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
				Gui::SameLine(0, style.ItemInnerSpacing.x);
				if (Gui::Button("...", vec2(buttonSize)))
				{
					if (chartEditor.OpenLoadAudioFileDialog())
						changesMade = true;
				}
				Gui::PopStyleVar();

				Gui::PopItemDisabledAndTextColorIf(songIsLoading);
				return false;
			});

			GuiProperty::TreeNode("Song", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::InputWithHint("Title", defaultHint, chart.Properties.Song.Title);
				changesMade |= GuiProperty::InputWithHint("Artist", defaultHint, chart.Properties.Song.Artist);
				changesMade |= GuiProperty::InputWithHint("Album", defaultHint, chart.Properties.Song.Album);
				changesMade |= GuiProperty::InputWithHint("Lyricist", defaultHint, chart.Properties.Song.Lyricist);
				changesMade |= GuiProperty::InputWithHint("Arranger", defaultHint, chart.Properties.Song.Arranger);
			});

			GuiProperty::TreeNode("Extra Info", ImGuiTreeNodeFlags_None, [&]
			{
				auto& extraInfo = chart.Properties.Song.ExtraInfo;
				for (size_t i = 0; i < extraInfo.size(); i++)
				{
					GuiPropertyRAII::ID id(&extraInfo[i]);
					changesMade |= GuiProperty::PropertyFuncValueFunc(
						[&] { GuiPropertyRAII::ItemWidth width(-1.0f); return Gui::InputTextWithHint("##Key", "Property", &extraInfo[i].Key); },
						[&] { GuiPropertyRAII::ItemWidth width(-1.0f); return Gui::InputTextWithHint("##Value", defaultHint, &extraInfo[i].Value); });
				}
			});

			GuiProperty::TreeNode("Chart Creator", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::InputWithHint("Name", defaultHint, chart.Properties.Creator.Name);
				changesMade |= GuiProperty::InputMultiline("Comment", chart.Properties.Creator.Comment, vec2(0.0f, 66.0f));
			});

			GuiProperty::TreeNode("Difficulty", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::Combo("Type", chart.Properties.Difficulty.Type, DifficultyNames);
				changesMade |= GuiProperty::Combo("Level", chart.Properties.Difficulty.Level, DifficultyLevelNames, ImGuiComboFlags_HeightLarge, static_cast<i32>(DifficultyLevel::StarMin), static_cast<i32>(DifficultyLevel::StarMax) + 1);
			});

			if (changesMade)
				undoManager.SetChangesWereMade();
		});
	}
}
