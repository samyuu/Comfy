#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Window/ApplicationHost.h"
#include "Tests/TestTask.h"
#include <numeric>
#include <atomic>

#if 0
std::atomic<size_t> DEBUG_GlobalAllocationCount;
std::atomic<size_t> DEBUG_GlobalAllocatedMemory;

void* operator new(size_t size)
{
	DEBUG_GlobalAllocationCount++;
	DEBUG_GlobalAllocatedMemory += size;

	return malloc(size);
}

void operator delete(void* pointer, size_t size)
{
	DEBUG_GlobalAllocationCount--;
	DEBUG_GlobalAllocatedMemory -= size;

	return free(pointer);
}

size_t GetAllocationCount() { return DEBUG_GlobalAllocationCount.load(); }
size_t GetAllocatedMemory() { return DEBUG_GlobalAllocatedMemory.load(); }
#else
size_t GetAllocationCount() { return 0u; }
size_t GetAllocatedMemory() { return 0u; }
#endif

namespace Comfy::Sandbox
{
	struct PerformanceOverlay
	{
		int Corner = 2;

		std::array<float, 512> ElapsedMSRingBuffer;
		int ElapsedRingBufferIndex = 0;

		void Gui(bool& isOpen)
		{
			constexpr float padding = 10.0f;

			const auto viewport = Gui::GetMainViewport();
			const auto position = vec2(
				(Corner & 1) ? (viewport->Pos.x + viewport->Size.x - padding) : (viewport->Pos.x + padding),
				(Corner & 2) ? (viewport->Pos.y + viewport->Size.y - padding) : (viewport->Pos.y + padding));

			const auto positionPivot = vec2(
				(Corner & 1) ? 1.0f : 0.0f,
				(Corner & 2) ? 1.0f : 0.0f);

			Gui::SetNextWindowPos(position, ImGuiCond_Always, positionPivot);
			Gui::SetNextWindowViewport(viewport->ID);
			Gui::SetNextWindowBgAlpha(0.85f);

			if (Gui::Begin("Performance Overlay", &isOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				const auto elapsedTime = TimeSpan::FromSeconds(Gui::GetIO().DeltaTime);

				if (ElapsedRingBufferIndex >= ElapsedMSRingBuffer.size())
					ElapsedRingBufferIndex = 0;
				ElapsedMSRingBuffer[ElapsedRingBufferIndex++] = static_cast<float>(elapsedTime.TotalMilliseconds());

				Gui::Text("Allocations: %zu Memory: %zu bytes", GetAllocationCount(), GetAllocatedMemory());

				Gui::Text("FPS: %.3f ms", Gui::GetIO().Framerate);
				Gui::Text("Elapsed Time: %.6f ms", elapsedTime.TotalMilliseconds());

				const auto averageMS = std::accumulate(ElapsedMSRingBuffer.begin(), ElapsedMSRingBuffer.end(), 0.0f) / static_cast<float>(ElapsedMSRingBuffer.size());

				char plotOverlayBuffer[64];
				sprintf_s(plotOverlayBuffer, "Average: %.6f ms", averageMS);

				Gui::Separator();
				Gui::PlotLines("##ElapsedTimePlot", ElapsedMSRingBuffer.data(), static_cast<int>(ElapsedMSRingBuffer.size()), 0, plotOverlayBuffer);

				if (Gui::BeginPopupContextWindow())
				{
					if (Gui::MenuItem("Top-left", nullptr, Corner == 0)) Corner = 0;
					if (Gui::MenuItem("Top-right", nullptr, Corner == 1)) Corner = 1;
					if (Gui::MenuItem("Bottom-left", nullptr, Corner == 2)) Corner = 2;
					if (Gui::MenuItem("Bottom-right", nullptr, Corner == 3)) Corner = 3;
					Gui::EndPopup();
				}
			}
			Gui::End();
		}
	};

	void Main()
	{
		ApplicationHost::ConstructionParam param = {};
		param.WindowTitle = "YEP COCK";
		param.IconHandle = nullptr;

		auto host = ApplicationHost(param);
		host.SetSwapInterval(1);

		bool showPerformanceOverlay = true;
		PerformanceOverlay performanceOverlay = {};

		std::unique_ptr<Tests::ITestTask> currentTestTask = nullptr; // std::make_unique<Tests::AetRendererTest>();

		host.EnterProgramLoop([&]
		{
			constexpr auto returnKey = Input::KeyCode_Escape;
			if (Gui::IsKeyPressed(returnKey, false))
				currentTestTask = nullptr;

			if (Gui::IsKeyPressed(Input::KeyCode_F1))
				host.SetSwapInterval(1);
			if (Gui::IsKeyPressed(Input::KeyCode_F2))
				host.SetSwapInterval(0);
			if (Gui::IsKeyPressed(Input::KeyCode_F3))
				showPerformanceOverlay ^= true;

			if (currentTestTask == nullptr)
			{
				const auto fullscreenWindowFlags = (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
				if (Gui::Begin("TestTaskSelection", nullptr, fullscreenWindowFlags))
				{
					Gui::TextUnformatted("Available ITestTasks:");
					Gui::SameLine();
					Gui::TextDisabled("(Return to TestTaskSelection by pressing '%s')", Input::GetKeyCodeName(returnKey));
					Gui::Separator();
					Tests::TestTaskInitializer::IterateRegistered([&](const auto& initializer)
					{
						if (Gui::Selectable(initializer.DerivedName))
							currentTestTask = initializer.Function();
					});
				}
				Gui::End();
			}
			else
			{
				currentTestTask->Update();
			}

			if (showPerformanceOverlay)
				performanceOverlay.Gui(showPerformanceOverlay);

			// Gui::DEBUG_NOSAVE_WINDOW("ShowStyleEditor", [&] { Gui::ShowStyleEditor(); }, ImGuiWindowFlags_None);
			// Gui::DEBUG_NOSAVE_WINDOW("ShowDemoWindow", [&] { Gui::ShowDemoWindow(); }, ImGuiWindowFlags_None);
		});
	}
}

int main(int argc, const char* argv[])
{
	Comfy::Sandbox::Main();
	return EXIT_SUCCESS;
}
