#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include <numeric>
#include <atomic>

#if 0
inline std::atomic<size_t> DEBUG_GlobalAllocationCount;
inline std::atomic<size_t> DEBUG_GlobalAllocatedMemory;

inline void* operator new(size_t size)
{
	DEBUG_GlobalAllocationCount++;
	DEBUG_GlobalAllocatedMemory += size;

	return malloc(size);
}

inline void operator delete(void* pointer, size_t size)
{
	DEBUG_GlobalAllocationCount--;
	DEBUG_GlobalAllocatedMemory -= size;

	return free(pointer);
}

inline size_t GetAllocationCount() { return DEBUG_GlobalAllocationCount.load(); }
inline size_t GetAllocatedMemory() { return DEBUG_GlobalAllocatedMemory.load(); }
#else
inline size_t GetAllocationCount() { return 0u; }
inline size_t GetAllocatedMemory() { return 0u; }
#endif

namespace Comfy::Sandbox
{
	struct PerformanceOverlay
	{
		int Corner = 2;

		std::array<float, 512> ElapsedMSRingBuffer;
		int ElapsedRingBufferIndex = 0;

		inline void Gui(bool& isOpen)
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

				Gui::Text("FPS: %.3f", Gui::GetIO().Framerate);
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
}
