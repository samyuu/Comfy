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

			GuiProperty::PropertyLabelValueFunc("Song File Name", [&]
			{
				const auto& style = Gui::GetStyle();
				const auto buttonSize = Gui::GetFrameHeight();

				const bool songIsLoading = chartEditor.IsSongAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(songIsLoading);

				Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvailWidth() - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));
				if (Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, (songIsLoading ? "Loading..." : defaultHint), &chart.SongFileName, ImGuiInputTextFlags_EnterReturnsTrue))
					chartEditor.LoadSongAsync(chart.SongFileName);
				Gui::PopItemWidth();

				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
				Gui::SameLine(0, style.ItemInnerSpacing.x);
				if (Gui::Button("...", vec2(buttonSize)))
					chartEditor.OpenLoadAudioFileDialog();
				Gui::PopStyleVar();

				Gui::PopItemDisabledAndTextColorIf(songIsLoading);
				return false;
			});

			GuiProperty::TreeNode("Song", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::InputWithHint("Title", defaultHint, chart.Properties.Song.Title);
				GuiProperty::InputWithHint("Artist", defaultHint, chart.Properties.Song.Artist);
				GuiProperty::InputWithHint("Album", defaultHint, chart.Properties.Song.Album);
				GuiProperty::InputWithHint("Lyricist", defaultHint, chart.Properties.Song.Lyricist);
				GuiProperty::InputWithHint("Arranger", defaultHint, chart.Properties.Song.Arranger);
			});

			GuiProperty::TreeNode("Creator", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::InputWithHint("Name", "Who asked? PogO", chart.Properties.Creator.Name);
				GuiProperty::InputMultiline("Comment", chart.Properties.Creator.Comment);
			});
		});
	}
}
