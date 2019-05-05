#include "TestTimeline.h"
#include "../TimeSpan.h"
#include "../Application.h"
#include "../Rendering/Texture.h"
#include "Chart/BpmChange.h"

TestTimeline::TestTimeline(Application* parent) : BaseWindow(parent)
{
}

TestTimeline::~TestTimeline()
{
}

const char* TestTimeline::GetGuiName()
{
	return u8"Test Timeline";
}


constexpr float BUTTON_STRIP_WIDTH = 46.0f;
constexpr float GRID_SZ = 64.0f;
constexpr float SCROLL_AMOUNT = 200.0;
constexpr float ICON_SIZE = 0.35f;
constexpr float ROW_HEIGHT = 42.0f;

enum TargetType
{
	TARGET_SANKAKU,
	TARGET_SHIKAKU,
	TARGET_BATSU,
	TARGET_MARU,
	TARGET_SLIDE_L,
	TARGET_SLIDE_R,
	TARGET_MAX
};

static Texture* icons[TARGET_MAX];
const char* iconPaths[TARGET_MAX] =
{
	"rom/spr/icon/btn_sankaku.png",
	"rom/spr/icon/btn_shikaku.png",
	"rom/spr/icon/btn_batsu.png",
	"rom/spr/icon/btn_maru.png",
	"rom/spr/icon/btn_slide_l.png",
	"rom/spr/icon/btn_slide_r.png",
};

struct TimelineRow
{
	TargetType Type;
	//float Height = ROW_HEIGHT;
};

struct BpmChanges
{
	void Add(BpmChange bpmChange)
	{
		bpmChanges.push_back(bpmChange);
	}

	BpmChange& GetAt(size_t index)
	{
		return bpmChanges.at(index);
	}

	size_t Count()
	{
		return bpmChanges.size();
	}

private:
	std::vector<BpmChange> bpmChanges;
};

class TimelineTimes
{
public:

	std::vector<TimeSpan> Times;

	TimeSpan GetTimeAt(TimelineTick tick)
	{
		return Times.at(tick.TotalTicks());
	}



	static TimelineTimes CalculateTimelineTimes(BpmChanges& bpmChanges, size_t barCount)
	{
		const size_t timeCount = barCount * TimelineTick::TICKS_PER_BAR;

		std::vector<TimeSpan> times(timeCount);
		{
			// the time of the last bpm change end
			double bpmEndTime = 0.0;

			const size_t bpmChangeCount = bpmChanges.Count();
			for (size_t b = 0; b < bpmChangeCount; b++)
			{
				BpmChange& bpmChange = bpmChanges.GetAt(b);

				const double beatDuration = (60.0 / bpmChange.Bpm);
				const double beatTickDuration = (beatDuration / TimelineTick::TICKS_PER_BEAT);

				bool onlyBpmChange = (bpmChangeCount == 1);
				bool lastBpmChange = (b == (bpmChangeCount - 1));

				const size_t timesStart = bpmChange.Tick.TotalTicks();
				const size_t timesCount = (onlyBpmChange || lastBpmChange) ? (times.size()) : (bpmChanges.GetAt(b + 1).Tick.TotalTicks());

				for (size_t i = 0, t = timesStart; t < timesCount; t++)
					times[t] = (beatTickDuration * i++) + bpmEndTime;

				bpmEndTime = times[timesCount - 1].Seconds() + beatTickDuration;
			}
		}

		TimelineTimes timelineTimes(times);

		//for (size_t i = 0; i <= 12 * TimelineTick::TICKS_PER_BAR; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromTicks(i)).Milliseconds());
		//for (size_t i = 0; i <= 12; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromBars(i)).Milliseconds());

		//for (size_t i = 0; i <= 12 * 4; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromTicks(i * TimelineTick::TICKS_PER_BEAT)).Milliseconds());

		return timelineTimes;
	}

private:
	TimelineTimes(std::vector<TimeSpan>& times) : Times(times) {};
};

// TODO: need some conversion between TimeStamp, TimelineTick and float TimelinePosition (which gets mulitplied by zoom)

constexpr int TIMELINE_ROWS = TARGET_MAX;
TimelineRow* timelineRows[TIMELINE_ROWS];

void TestTimeline::DrawGui()
{
	// TIME TEST:
	static bool timeTest = true;
	if (timeTest)
	{
		timeTest = false;
		
		BpmChanges bpmChanges;
		bpmChanges.Add(BpmChange(TimelineTick::FromBars(0), 180.0f));
		bpmChanges.Add(BpmChange(TimelineTick::FromBars(4), 160.0f));
		bpmChanges.Add(BpmChange(TimelineTick::FromBars(10), 200.0f));

		TimelineTimes times = TimelineTimes::CalculateTimelineTimes(bpmChanges, 200);
	}

	ImU32 GRID_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_Separator]);
	auto _gridColor = ImGui::GetStyle().Colors[ImGuiCol_Separator]; _gridColor.w *= .5f;
	ImU32 GRID_COLOR_ALT = ImColor(_gridColor);
	ImU32 BUTTON_STRIP_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_Separator]);
	ImU32 SELECTION_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg]);
	ImU32 TIMELINE_ROW_BG_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg]);


	for (size_t i = 0; i < TARGET_MAX; i++)
		if (icons[i] == nullptr) { icons[i] = new Texture(); icons[i]->LoadFromFile(iconPaths[i]); }

	if (false)
		ImGui::ShowDemoWindow(nullptr);

	static float cursorPos;
	static ImRect dragRect;

	ImGui::BeginGroup();
	{
		static char timeInputBuffer[32] = "00:00:000";
		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time", "00:00:000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Button("Stop");
		ImGui::SameLine();
		ImGui::Button("Pause");
		ImGui::SameLine();
		ImGui::Button("Play");
		ImGui::SameLine();
		ImGui::Button("|<");
		ImGui::SameLine();
		ImGui::Button(">|");
	}
	ImGui::EndGroup();

	ImGui::BeginChild("##timeline_child", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

		ImVec2 viewTopLeft = ImGui::GetWindowPos() + ImGui::GetCursorPos();

		// Scroll Test
		// -----------
		{
			ImGui::ItemSize(ImVec2(1000, 0));
			//ImGui::BeginGroup();
			//window->ScrollbarY = 1000;
			//ImGui::ItemSize(ImVec2(10000, 0));
			//window->SizeContents.y = 10000;
			//ImGui::EndGroup();
		}


		// First draw the timeline row lines
		// ---------------------------------
		for (int i = 0; i <= TIMELINE_ROWS; i++)
		{
			TimelineRow* timelineRow;

			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + viewTopLeft;
			auto end = ImVec2(ImGui::GetWindowWidth(), y) + viewTopLeft;

			// Draw timeline row separator
			// ---------------------------
			windowDrawList->AddLine(start, end, GRID_COLOR);

			// Draw timeline row background
			// ----------------------------
			if (i < TIMELINE_ROWS)
			{
				start.y += 1;
				end.y -= 1;
				end.y += ROW_HEIGHT;
				windowDrawList->AddRectFilled(start, end, TIMELINE_ROW_BG_COLOR);
			}
		}

		// Then draw the buttonstrip
		// -------------------------
		{
			windowDrawList->AddRectFilled(viewTopLeft, viewTopLeft + ImVec2(BUTTON_STRIP_WIDTH, ImGui::GetWindowHeight()), BUTTON_STRIP_COLOR);

			// And the icons
			// -------------
			for (int i = 0; i < TIMELINE_ROWS; i++)
			{
				float y = i * ROW_HEIGHT;
				auto start = ImVec2(0, y) + viewTopLeft;
				auto end = ImVec2(BUTTON_STRIP_WIDTH, y) + viewTopLeft;

				start.y += 1;
				start.x += 1;
				end.y += ROW_HEIGHT;
				end.x -= 1;
				windowDrawList->AddRectFilled(start, end, IM_COL32(0, 0, 0, 16));
				ImGui::AddTexture(windowDrawList, icons[i], ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f), ICON_SIZE);
			}

			viewTopLeft += ImVec2(BUTTON_STRIP_WIDTH, 0);
		}

		//static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
		//if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		//	scrolling = scrolling + ImGui::GetIO().MouseDelta;
		//if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::GetIO().MouseWheel != 0.0f)
		//	scrolling.x -= ImGui::GetIO().MouseWheel * SCROLL_AMOUNT;

		//ImVec2 win_pos = ImGui::GetCursorScreenPos();
		//ImVec2 canvas_sz = ImGui::GetWindowSize();

		//windowDrawList->AddLine(ImVec2(0.0f, 0) + win_pos, ImVec2(canvas_sz.x, 0) + win_pos, GRID_COLOR);
		//for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
		//	windowDrawList->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);

		// Test out bar dividors
		// ---------------------
		{
			float barWidth = 48;
			int i = 0;
			for (float x = 0.0f; x <= ImGui::GetWindowWidth(); x += barWidth)
			{
				auto start = ImVec2(x, 0) + viewTopLeft;
				auto end = ImVec2(x, ImGui::GetWindowHeight()) + viewTopLeft;

				windowDrawList->AddLine(start, end, i++ % 2 == 0 ? GRID_COLOR : GRID_COLOR_ALT);
			}
		}

		// Test out timline buttons
		// ------------------------
		{
			static int target;
			if (ImGui::IsKeyPressed(GLFW_KEY_UP)) target++;
			if (ImGui::IsKeyPressed(GLFW_KEY_DOWN)) target--;
			if (target > TARGET_SLIDE_R) target = TARGET_SANKAKU;
			if (target < TARGET_SANKAKU) target = TARGET_SLIDE_R;


			ImGui::AddTexture(windowDrawList, icons[target], viewTopLeft + ImVec2(0, ROW_HEIGHT / 2), ICON_SIZE);
		}

		// Test out selection box
		// ----------------------
		{
			if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringWindow() && !ImGui::IsAnyItemHovered())
				dragRect.Min = ImGui::GetMousePos();
			if (ImGui::IsMouseReleased(0))
				dragRect.Min = dragRect.Max = ImVec2();

			if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseDragging(0) && dragRect.Min.x != 0)
			{
				dragRect.Max = ImGui::GetMousePos();
				windowDrawList->AddRectFilled(dragRect.GetTL(), dragRect.GetBR(), SELECTION_COLOR);
			}
		}


		// Scroll Control
		// --------------
		{
			if (ImGui::GetIO().MouseWheel != 0.0f && !ImGui::GetIO().KeyCtrl)
			{
				ImVec2 max_step = (window->ContentsRegionRect.GetSize() + window->WindowPadding * 2.0f) * 0.67f;

				if (ImGui::GetIO().KeyShift)
				{
					float scroll_step = ImFloor(ImMin(5 * window->CalcFontSize(), max_step.y));
					ImGui::SetWindowScrollY(window, window->Scroll.y - ImGui::GetIO().MouseWheel * scroll_step);
				}
				else if (!ImGui::GetIO().KeyShift)
				{
					float scroll_step = ImFloor(ImMin(2 * window->CalcFontSize(), max_step.x));
					ImGui::SetWindowScrollX(window, window->Scroll.x + ImGui::GetIO().MouseWheel * scroll_step);
				}
			}
		}


	}
	ImGui::EndChild();
}

ImGuiWindowFlags TestTimeline::GetWindowFlags()
{
	return ImGuiWindowFlags_None;
}