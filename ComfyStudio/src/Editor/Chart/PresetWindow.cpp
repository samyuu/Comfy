#include "PresetWindow.h"
#include "ChartEditor.h"
#include "ChartCommands.h"
#include "TargetPropertyRules.h"
#include "Core/ComfyStudioApplication.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include <FontIcons.h>

// TODO: Implement a "Curve Placement Tool" instead (?) similar to how bezier curves can be edited in blender
#define COMFY_BEZIER_TEST 0

#if COMFY_BEZIER_TEST
#include "Misc/BezierCurve.h"

// TODO: Try to accurately recreate patterns in official charts, test what distance factors they use
/*
- seperate "fullscreen" popup window for editing the bezier paths to reduce dependencies to and from other systems
- list of selecable paths, just like for sync presets with button hover preview
- helper menu options for forming circle paths etc.
- zoom in
- multi keypoint selection like blender
- preset independent beat offset to be applied to all selected targets before calculating properties
curve path distance *should* be based on the curve distance not of the finite sampled points targets from one to another (?)
- different placement options, either position targets based on their correct beat distance *or* space all selected targets evenly along curve path (?)
- preset option to round position to nearest pixel and angle to nearest whole (?)

... maybe special case small/large circles, as they require perfect precision (?)
*/

/*
#Comfy::Studio::ChartEditor Clipboard 9f88783c
Target { 192 3 1 0 0 0 1440.00 504.00 175.00 -2.00 500.00 1440.00 };
Target { 240 2 1 0 0 0 1248.00 504.00 173.00 -2.00 500.00 1440.00 };
Target { 276 1 1 0 0 0 1104.00 504.00 171.50 -2.00 500.00 1440.00 };
Target { 288 1 1 0 0 0 1056.00 504.00 171.00 -2.00 500.00 1440.00 };
Target { 312 1 1 0 0 0 960.00 504.00 170.00 -2.00 500.00 1440.00 };
Target { 336 1 1 0 0 0 868.00 532.00 60.00 2.00 500.00 1440.00 };
Target { 360 1 1 0 0 0 808.00 604.00 30.00 2.00 500.00 1440.00 };
Target { 384 0 1 0 0 0 792.00 696.00 0.00 2.00 500.00 1440.00 };
Target { 408 0 1 0 0 0 832.00 784.00 -30.00 2.00 500.00 1440.00 };
Target { 432 1 1 0 0 0 912.00 832.00 -60.00 2.00 500.00 1440.00 };
Target { 456 1 1 0 0 0 1008.00 832.00 -90.00 2.00 500.00 1440.00 };
Target { 480 2 1 0 0 0 1088.00 784.00 -120.00 2.00 500.00 1440.00 };
Target { 504 2 1 0 0 0 1128.00 696.00 -150.00 2.00 500.00 1440.00 };
Target { 528 3 1 0 0 0 1112.00 604.00 -180.00 2.00 500.00 1440.00 };
Target { 552 3 1 0 0 0 1052.00 532.00 -210.00 2.00 500.00 1440.00 };
Target { 576 0 1 1 0 0 960.00 216.00 -35.00 0.00 500.00 1152.00 };
Target { 576 3 1 1 0 0 960.00 504.00 -145.00 0.00 500.00 1152.00 };
*/

/*
pv_250 curve patterns:

#Comfy::Studio::ChartEditor Clipboard 9f88783c
Target { 10273 0 1 0 0 0 668.24 332.72 165.00 2.00 500.00 1212.00 };
Target { 10297 0 1 0 0 0 736.12 264.84 170.00 2.00 500.00 1212.00 };
Target { 10321 0 1 0 0 0 819.28 216.84 175.00 2.00 500.00 1212.00 };
Target { 10345 0 1 0 0 0 912.00 192.00 180.00 2.00 500.00 1212.00 };
Target { 10369 0 1 0 0 0 1008.00 192.00 185.00 2.00 500.00 1212.00 };
Target { 10393 0 1 0 0 0 1100.72 216.84 190.00 2.00 500.00 1212.00 };
Target { 10417 0 1 0 0 0 1183.88 264.84 195.00 2.00 500.00 1212.00 };
Target { 10441 0 1 0 0 0 1251.76 332.72 200.00 2.00 500.00 1212.00 };

#Comfy::Studio::ChartEditor Clipboard 9f88783c
Target { 20833 0 1 0 0 0 668.24 723.28 15.00 -2.00 500.00 1212.00 };
Target { 20857 0 1 0 0 0 736.12 791.16 10.00 -2.00 500.00 1212.00 };
Target { 20881 0 1 0 0 0 819.28 839.16 5.00 -2.00 500.00 1212.00 };
Target { 20905 0 1 0 0 0 912.00 864.00 0.00 -2.00 500.00 1212.00 };
Target { 20929 0 1 0 0 0 1008.00 864.00 -5.00 -2.00 500.00 1212.00 };
Target { 20953 0 1 0 0 0 1100.72 839.16 -10.00 -2.00 500.00 1212.00 };
Target { 20977 0 1 0 0 0 1183.88 791.16 -15.00 -2.00 500.00 1212.00 };
Target { 21001 0 1 0 0 0 1251.76 723.28 -20.00 -2.00 500.00 1212.00 };


pv_817_hard small circle:

#Comfy::Studio::ChartEditor Clipboard 9f88783c
Target { 14054 3 1 0 0 0 960.00 390.49 0.00 -2.00 500.00 1400.00 };
Target { 14078 3 1 0 0 0 1048.39 422.66 40.00 -2.00 500.00 1400.00 };
Target { 14102 3 1 0 0 0 1095.42 504.12 80.00 -2.00 500.00 1400.00 };
Target { 14126 3 1 0 0 0 1079.08 596.75 120.00 -2.00 500.00 1400.00 };
Target { 14150 2 1 0 0 0 1007.03 657.22 160.00 -2.00 500.00 1400.00 };
Target { 14174 2 1 0 0 0 912.97 657.22 200.00 -2.00 500.00 1400.00 };
Target { 14198 2 1 0 0 0 840.91 596.75 240.00 -2.00 500.00 1400.00 };
Target { 14222 2 1 0 0 0 824.58 504.12 280.00 -2.00 500.00 1400.00 };
Target { 14246 1 1 0 0 0 871.61 422.66 320.00 -2.00 500.00 1400.00 };
*/

namespace Comfy::Studio::Editor
{
	namespace BezierTestNew
	{
		static ImRect DEBUG_RENDER_REGION;

		static f32 TestSequenceOverlayDimmness = 0.15f; // 0.75;
		static std::vector<vec2> CirclePositionsToPreview;

		struct BezierKey
		{
			vec2 Point, ControlStart, ControlEnd;
			// f32 DistanceScale;

			vec2& operator[](size_t index) { assert(index < (sizeof(BezierKey) / sizeof(vec2))); return (&Point)[index]; }
			vec2 operator[](size_t index) const { assert(index < (sizeof(BezierKey) / sizeof(vec2))); return (&Point)[index]; }
		};

		struct BezierPath
		{
			BeatTick LoopDuration;
			// NOTE: Closed curve "Cyclic", as blender calls it
			bool Cyclic;
			std::vector<BezierKey> Keys;

			// TODO: Precalc distance -> t map (?)
		};

		void DebugDrawBezierPath(const BezierPath& bezierPath, ImDrawList* drawList, ImRect windowRect)
		{
			constexpr f32 lineFillStep = 0.001f;
			auto toScreenSpace = [windowRect](vec2 point) { return windowRect.GetTL() + ((point / Rules::PlacementAreaSize) * windowRect.GetSize()); };

			const auto& keys = bezierPath.Keys;

			if (keys.size() > 1)
			{
				for (size_t i = 1; i < keys.size(); i++)
				{
					const auto& lastKey = keys[i - 1];
					const auto& thisKey = keys[i - 0];

					for (f32 t = 0.0f; t <= 1.0f; t += lineFillStep)
						drawList->PathLineTo(toScreenSpace(CubicBezier::GetPoint(lastKey.Point, thisKey.Point, lastKey.ControlEnd, thisKey.ControlStart, t)));
				}

				if (bezierPath.Cyclic)
				{
					const auto& lastKey = keys.back();
					const auto& thisKey = keys.front();

					for (f32 t = 0.0f; t <= 1.0f; t += lineFillStep)
						drawList->PathLineTo(toScreenSpace(CubicBezier::GetPoint(lastKey.Point, thisKey.Point, lastKey.ControlEnd, thisKey.ControlStart, t)));
				}

				drawList->PathStroke(ImColor(0.0f, 1.0f, 0.0f, 0.85f), false, 2.0f);
			}

			for (const auto& key : keys)
			{
				drawList->AddLine(toScreenSpace(key.Point), toScreenSpace(key.ControlStart), ImColor(0x6968DFFE), 1.0f);
				drawList->AddLine(toScreenSpace(key.Point), toScreenSpace(key.ControlEnd), ImColor(0x6968DFFE), 1.0f);

				drawList->AddCircleFilled(toScreenSpace(key.Point), 6.0f, ImColor(1.0f, 0.0f, 0.0f));
				drawList->AddCircleFilled(toScreenSpace(key.ControlStart), 4.0f, ImColor(0.5f, 0.0f, 0.0f));
				drawList->AddCircleFilled(toScreenSpace(key.ControlEnd), 4.0f, ImColor(0.5f, 0.0f, 0.0f));
			}


#if 1
			if (keys.size() > 1)
			{
				constexpr f32 distCalcStep = 0.0001f;
				CirclePositionsToPreview.clear();

				f32 distanceTraversed = 0.0f;

				vec2 lastP = CubicBezier::GetPoint(keys[0].Point, keys[1].Point, keys[0].ControlEnd, keys[1].ControlStart, 0.0f);
				CirclePositionsToPreview.push_back(/*toScreenSpace*/(lastP));

				auto processKeyPair = [&](const BezierKey& lastKey, const BezierKey& thisKey)
				{
					for (f32 t = distCalcStep; t <= 1.0f; t += distCalcStep)
					{
						const vec2 p = CubicBezier::GetPoint(lastKey.Point, thisKey.Point, lastKey.ControlEnd, thisKey.ControlStart, t);
						const f32 dist = glm::distance(p, lastP);

						lastP = p;

						constexpr f32 targetDist = Rules::TickToDistance(BeatTick::FromBars(1) / 8);
						if (distanceTraversed >= targetDist)
						{
							distanceTraversed -= targetDist;
							CirclePositionsToPreview.push_back(/*toScreenSpace*/(p));
						}

						distanceTraversed += dist;
					}
				};

				for (size_t i = 1; i < keys.size(); i++)
				{
					const auto& lastKey = keys[i - 1];
					const auto& thisKey = keys[i - 0];

					processKeyPair(lastKey, thisKey);
				}

				if (bezierPath.Cyclic)
				{
					const auto& lastKey = keys.back();
					const auto& thisKey = keys.front();

					processKeyPair(lastKey, thisKey);
				}
			}
#endif
		}

		void DoGuiTest()
		{
			const auto windowRect = BezierTestNew::DEBUG_RENDER_REGION;
			auto toScreenSpace = [windowRect](vec2 worldSpace) { return windowRect.GetTL() + (worldSpace * windowRect.GetSize()); };
			auto toWorldSpace = [windowRect](vec2 screenSpace) { return (screenSpace - windowRect.GetTL()) * (Rules::PlacementAreaSize / windowRect.GetSize()); };

			static BezierTestNew::BezierPath testBezierPath = []()
			{
				BezierTestNew::BezierPath path = {};
				path.Keys =
				{
					BezierTestNew::BezierKey { vec2(336.0f, 432.0f), vec2(336.0f, 432.0f), vec2(480.0f, 768.0f) },
					BezierTestNew::BezierKey { vec2(1584.0f, 432.0f), vec2(1440.0f, 768.0f), vec2(1440.0f, 968.0f) },
					BezierTestNew::BezierKey { vec2(1284.0f, 232.0f), vec2(1440.0f, 768.0f), vec2(1440.0f, 768.0f) },
				};
				return path;
			}();

			static f32 testCircleRadius = (576.0f / 2.0f);

			if (Gui::DragFloat("Circle Radius", &testCircleRadius) | Gui::Button("Circle"))
			{
				constexpr vec2 DynamicSequencePresetLargeCircleCenter = Rules::PlacementAreaCenter;

				/*constexpr*/ f32 radiusHorizontal = testCircleRadius; // (Rules::TickToDistance((BeatTick::FromBars(1) / 16) * 2) * glm::pi<f32>() / 2.0f); // 
				/*constexpr*/ f32 radiusVertical = radiusHorizontal;
				constexpr f32 n = 4.0f;
				// https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
				const f32 optimalLengthUnit = (4.0f / 3.0f) * glm::tan(glm::pi<f32>() / (2.0f * n));
				const f32 optimalLengthHorizontal = optimalLengthUnit * radiusHorizontal;
				const f32 optimalLengthVertical = optimalLengthUnit * radiusVertical;

				testBezierPath.Cyclic = true;
				testBezierPath.Keys.clear();
				testBezierPath.Keys.push_back({ vec2(0.0f, -radiusVertical), vec2(), vec2() });
				testBezierPath.Keys.push_back({ vec2(+radiusHorizontal, 0.0f), vec2(), vec2() });
				testBezierPath.Keys.push_back({ vec2(0.0f, +radiusVertical), vec2(), vec2() });
				testBezierPath.Keys.push_back({ vec2(-radiusHorizontal, 0.0f), vec2(), vec2() });

				testBezierPath.Keys[0].ControlStart = testBezierPath.Keys[0].Point + vec2(-optimalLengthVertical, 0.0f);
				testBezierPath.Keys[0].ControlEnd = testBezierPath.Keys[0].Point + vec2(+optimalLengthVertical, 0.0f);

				testBezierPath.Keys[1].ControlStart = testBezierPath.Keys[1].Point + vec2(0.0f, -optimalLengthHorizontal);
				testBezierPath.Keys[1].ControlEnd = testBezierPath.Keys[1].Point + vec2(0.0f, +optimalLengthHorizontal);

				testBezierPath.Keys[2].ControlStart = testBezierPath.Keys[2].Point + vec2(+optimalLengthVertical, 0.0f);
				testBezierPath.Keys[2].ControlEnd = testBezierPath.Keys[2].Point + vec2(-optimalLengthVertical, 0.0f);

				testBezierPath.Keys[3].ControlStart = testBezierPath.Keys[3].Point + vec2(0.0f, +optimalLengthHorizontal);
				testBezierPath.Keys[3].ControlEnd = testBezierPath.Keys[3].Point + vec2(0.0f, -optimalLengthHorizontal);

				//testBezierPath.Keys.push_back(testBezierPath.Keys[0]);

				for (auto& key : testBezierPath.Keys)
				{
					for (i32 i = 0; i < 3; i++)
						key[i] += DynamicSequencePresetLargeCircleCenter;
				}
			}

			if (Gui::Button("Pop Key"))
			{
				if (!testBezierPath.Keys.empty())
					testBezierPath.Keys.pop_back();
			}
			Gui::SameLine();
			if (Gui::Button("Clear Keys"))
			{
				testBezierPath.Keys.clear();
			}

			Gui::Checkbox("Cyclic", &testBezierPath.Cyclic);

			static struct GrabData
			{
				i32 KeyIndex = -1;
				i32 PropertyIndex = -1;
				bool SplitAnchor = false;
			} testGrab;

			static vec2 worldMousePos, worldMousePosLast;
			worldMousePosLast = worldMousePos;
			worldMousePos = toWorldSpace(Gui::GetMousePos());
			const vec2 worldMouseDelta = (worldMousePos - worldMousePosLast);

			//if (windowRect.Contains(Gui::GetMousePos()) && Gui::IsMouseClicked(ImGuiMouseButton_Left))
			if (Gui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && Gui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				testGrab = {};
				for (i32 k = 0; k < static_cast<i32>(testBezierPath.Keys.size()); k++)
				{
					const auto& key = testBezierPath.Keys[k];
					for (i32 p = 0; p < 3; p++)
					{
						if (glm::distance(worldMousePos, key[p]) < 8.0f)
						{
							testGrab = { k, p };
							testGrab.SplitAnchor = Gui::GetIO().KeyAlt;
						}
					}
				}
			}

			if (Gui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && Gui::IsMouseClicked(ImGuiMouseButton_Middle))
			{
				testGrab = {};
				const vec2 snappedMouse = Rules::SnapPositionToGrid(worldMousePos);
				const vec2 newKeyPos = snappedMouse;

				if (/*testBezierPath.Keys.empty()*/testBezierPath.Keys.size() < 2)
				{
					testBezierPath.Keys.push_back({ newKeyPos, newKeyPos, newKeyPos });
				}
				else
				{
					auto& lastKey = testBezierPath.Keys.back();
					auto& secondToLastKey = testBezierPath.Keys[testBezierPath.Keys.size() - 2];

					const f32 distanceToLastKey = glm::distance(lastKey.Point, newKeyPos);

					// TODO: Find mid point, move into line normal direction, control end & stat pointing towards extruded midpoint scaled by size
					//const vec2 directionFromLastKey = glm::normalize(lastKey.Point - newKeyPos);
					//const vec2 directionToLastKey = glm::normalize(newKeyPos - lastKey.Point);
					const vec2 directionFromLastKey = glm::normalize(lastKey.Point - newKeyPos);
					const vec2 directionToLastKey = glm::normalize(newKeyPos - secondToLastKey.Point);

					constexpr f32 n = 4.0f;
					const f32 radius = distanceToLastKey * 0.5f;
					const f32 optimalLength = (4.0f / 3.0f) * glm::tan(glm::pi<f32>() / (2.0f * n)) * radius;

					lastKey.ControlEnd = lastKey.Point + (directionToLastKey * optimalLength);

					testBezierPath.Keys.push_back({ newKeyPos, newKeyPos + (directionFromLastKey * optimalLength), newKeyPos });
				}
			}

			if (Gui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				testGrab = {};
			}

			if (InBounds(testGrab.KeyIndex, testBezierPath.Keys))
			{
				Gui::SetActiveID(Gui::GetID(&testGrab), Gui::GetCurrentWindow());

				auto& grabbedKey = testBezierPath.Keys[testGrab.KeyIndex];
				if (testGrab.PropertyIndex == 0)
				{
					for (i32 i = 0; i < 3; i++)
						grabbedKey[i] += worldMouseDelta;
				}
				else
				{
					if (testGrab.SplitAnchor)
					{
						grabbedKey[testGrab.PropertyIndex] = worldMousePos;
					}
					else
					{
						auto& mainKeyControl = grabbedKey[testGrab.PropertyIndex];
						auto& secondaryKeyControl = grabbedKey[testGrab.PropertyIndex == 1 ? 2 : 1];

						mainKeyControl = worldMousePos;
						secondaryKeyControl = -(mainKeyControl - grabbedKey.Point) + grabbedKey.Point;
					}
				}
			}

			BezierTestNew::DebugDrawBezierPath(testBezierPath, Gui::GetForegroundDrawList(), windowRect);
		}
	}
}
#endif

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr f32 MaxDimRender = 0.15f, MaxDimOverlay = 0.45f;
		constexpr f32 HoverFadeInMS = 60.0f, HoverFadeOutMS = 75.0f;

		constexpr vec2 PresetButtonSpacing = vec2(2.0f);

		constexpr f32 DynamicSyncButtonHeight = 44.0f;
		constexpr f32 StaticSyncButtonHeight = 22.0f;
		constexpr f32 SyncSettingsButtonWidth = 26.0f;

		constexpr f32 SingleLineSequenceButtonHeight = StaticSyncButtonHeight;
		constexpr f32 SameLineSequenceButtonHeight = StaticSyncButtonHeight;
	}

	PresetWindow::PresetWindow(ChartEditor& chartEditor, Undo::UndoManager& undoManager) : chartEditor(chartEditor), undoManager(undoManager)
	{
		// sequencePresetSettings.TickOffset = BeatTick::FromBeats(2);
		sequencePresetSettings.ApplyFirstTargetTickAsOffset = true;
	}

	void PresetWindow::SyncGui(Chart& chart)
	{
		hovered.Sync.DynamicPreset = {};
		hovered.Sync.StaticPreset = {};
		hovered.Sync.AnyChildWindow = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		hovered.Sync.ContextMenu = false;
		hovered.Sync.ContextMenuOpen = false;

		const auto& style = Gui::GetStyle();
		const auto presetSettingsContextMenuID = Gui::GetCurrentWindow()->GetID("PresetWindowSyncSettingsContextMenu");

		const bool anySyncTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected && t.Flags.IsSync); });

		// TODO: Correctly factor in window padding and other missing stlye vars (?)
		const f32 dynamicChildHeight = (DynamicSyncButtonHeight + PresetButtonSpacing.y) * 3.0f + (style.WindowPadding.y * 2.0f) - PresetButtonSpacing.y;
		const f32 addChildHeight = (StaticSyncButtonHeight + PresetButtonSpacing.y + style.WindowPadding.y);
		const f32 minStaticChildHeight = ((StaticSyncButtonHeight + PresetButtonSpacing.y) * 2.5f);
		const f32 staticChildHeight = Max(addChildHeight + dynamicChildHeight + minStaticChildHeight + (style.WindowPadding.y * 2.0f), Gui::GetContentRegionAvail().y) - addChildHeight - dynamicChildHeight - (style.WindowPadding.y * 2.0f);

		Gui::BeginChild("DynamicSyncPresetsChild", vec2(0.0f, dynamicChildHeight), true);
		{
			hovered.Sync.DynamincChildWindow = Gui::IsWindowHovered();

			const f32 halfWidth = (Gui::GetContentRegionAvail().x - PresetButtonSpacing.x) / 2.0f;
			std::array<ImRect, EnumCount<DynamicSyncPreset>()> presetIconRectsToDraw;

			auto dynamicSyncPresetButton = [&](DynamicSyncPreset preset)
			{
				const i32 presetIndex = static_cast<i32>(preset);
				Gui::PushID(presetIndex);
				
				// NOTE: Only render the actual text as a fallback in case the icons failed to load
				const char* buttonLabel = (sprites.DynamicSyncPresetIcons[presetIndex] != nullptr) ? "##DynamicSyncPresetButton" : DynamicSyncPresetNames[presetIndex];

				if (Gui::ButtonEx(buttonLabel, vec2(halfWidth, DynamicSyncButtonHeight)))
					ApplyDynamicSyncPresetToSelectedTargets(undoManager, chart, preset, dynamicSyncPresetSettings);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.Sync.DynamicPreset = preset;

				const auto rect = [r = Gui::FitFixedAspectRatio(Gui::GetCurrentWindowRead()->DC.LastItemRect, 1.0f)]() mutable { r.Expand(-4.0f); return r; }();
				presetIconRectsToDraw[static_cast<size_t>(preset)] = rect;
			};

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, PresetButtonSpacing);
			{
				Gui::PushItemDisabledAndTextColorIf(!anySyncTargetSelected);
				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i += 2)
				{
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 0));
					Gui::SameLine();
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 1));
				}

				// NOTE: Delay icon rendering to optimize texture batching
				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i++)
				{
					const ImRect iconRect = presetIconRectsToDraw[i];
					const auto* iconSpr = sprites.DynamicSyncPresetIcons[i];

					if (iconSpr != nullptr)
						Gui::AddSprite(Gui::GetWindowDrawList(), *editorSprites, *iconSpr, iconRect.GetTL(), iconRect.GetBR(), Gui::GetColorU32(ImGuiCol_Text));
				}
				Gui::PopItemDisabledAndTextColorIf(!anySyncTargetSelected);
			}
			Gui::PopStyleVar();
		}
		Gui::EndChild();

		Gui::BeginChild("StaticSyncPresetsChild", vec2(0.0f, staticChildHeight), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			hovered.Sync.StaticChildWindow = Gui::IsWindowHovered();

			for (const auto& staticSyncPreset : GlobalUserData.TargetPreset.StaticSyncPresets)
			{
				const bool presetDisabled = (!anySyncTargetSelected || staticSyncPreset.TargetCount < 1);
				Gui::PushID(&staticSyncPreset);
				Gui::PushItemDisabledAndTextColorIf(presetDisabled);

				if (Gui::ButtonEx(staticSyncPreset.Name.c_str(), vec2(Gui::GetContentRegionAvail().x, StaticSyncButtonHeight)))
					ApplyStaticSyncPresetToSelectedTargets(undoManager, chart, staticSyncPreset);

				Gui::PopItemDisabledAndTextColorIf(presetDisabled);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.Sync.StaticPreset = static_cast<size_t>(std::distance(&*GlobalUserData.TargetPreset.StaticSyncPresets.cbegin(), &staticSyncPreset));

				// TODO: At least basic item context menu for changing the name, move up/down and delete (?)
			}
		}
		Gui::EndChild();

		Gui::BeginChild("AddPresetsChild", vec2(0.0f, addChildHeight), true);
		{
			hovered.Sync.AddChildWindow = Gui::IsWindowHovered();

			const bool addNewEnabled = COMFY_DEBUG_RELEASE_SWITCH(anySyncTargetSelected, false);
			Gui::PushItemDisabledAndTextColorIf(!addNewEnabled);

			if (Gui::ButtonEx("Add New...", vec2(Gui::GetContentRegionAvail().x - SyncSettingsButtonWidth, StaticSyncButtonHeight)))
			{
#if COMFY_DEBUG && 1 // TODO:
				if (const auto firstSelectedTarget = FindIfOrNull(chart.Targets.GetRawView(), [&](auto& t) { return (t.IsSelected && t.Flags.IsSync); }); firstSelectedTarget != nullptr)
				{
					const auto syncPair = &firstSelectedTarget[-firstSelectedTarget->Flags.IndexWithinSyncPair];
					assert(syncPair[0].Flags.IndexWithinSyncPair == 0);

					auto& newPreset = GlobalUserData.Mutable().TargetPreset.StaticSyncPresets.emplace_back();
					newPreset.Name = "Unnamed Preset " + std::to_string(GlobalUserData.TargetPreset.StaticSyncPresets.size());
					newPreset.TargetCount = syncPair->Flags.SyncPairCount;
					for (size_t i = 0; i < newPreset.TargetCount; i++)
					{
						newPreset.Targets[i].Type = syncPair[i].Type;
						newPreset.Targets[i].Properties = Rules::TryGetProperties(syncPair[i]);
					}

					GlobalUserData.Mutable().SaveToFile();
				}
#endif
			}

			Gui::PopItemDisabledAndTextColorIf(!addNewEnabled);

			Gui::SameLine(0.0f, 0.0f);
			if (Gui::Button(ICON_FA_COG, vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				Gui::OpenPopupEx(presetSettingsContextMenuID);
		}
		Gui::EndChild();

		if (Gui::IsMouseReleased(ImGuiMouseButton_Right) && (hovered.Sync.AnyChildWindow && Gui::WasHoveredWindowHoveredOnMouseClicked(ImGuiMouseButton_Right)) && Gui::IsMouseSteady())
			Gui::OpenPopupEx(presetSettingsContextMenuID);

		if (Gui::BeginPopupEx(presetSettingsContextMenuID, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking)))
		{
			hovered.Sync.ContextMenu = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			hovered.Sync.ContextMenuOpen = true;

			static constexpr std::array checkboxBindings =
			{
				Input::Binding(Input::KeyCode_1), Input::Binding(Input::KeyCode_2), Input::Binding(Input::KeyCode_3),
				Input::Binding(Input::KeyCode_4), Input::Binding(Input::KeyCode_5), Input::Binding(Input::KeyCode_6),
				Input::Binding(Input::KeyCode_7), Input::Binding(Input::KeyCode_8), Input::Binding(Input::KeyCode_9),
			};

			auto checkbox = [](const char* label, bool& inOutBool, u32 checkboxIndex, const char* description = nullptr)
			{
				const auto* binding = IndexOrNull(checkboxIndex, checkboxBindings);
				Gui::MenuItemDontClosePopup(label, (binding != nullptr) ? Input::ToString(*binding).data() : nullptr, &inOutBool);

				if (binding != nullptr && Gui::IsWindowFocused() && Input::IsPressed(*binding, false))
					inOutBool ^= true;

				// NOTE: Extra long delay time to hopefully not get in the way
				if (description != nullptr && !inOutBool && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, 1.5f))
					Gui::WideSetTooltip(description);
			};

			u32 checkboxIndex = 0;
			Gui::TextUnformatted("Dynamic Sync Preset Settings:\t\t\t\t");
			Gui::Separator();
			checkbox("Steep Angles", dynamicSyncPresetSettings.SteepAngles, checkboxIndex++,
				"Applies to vertical and horizontal sync presets\n"
				"- Use steeper 35 instead of 45 degree vertical angles\n"
				"- Use steeper 10 instead of 20 degree horizontal angles\n"
				"Intended for\n"
				"- Sync pairs placed in quick succession\n"
				"- Sync pairs positioned closely to the top / bottom edge of the screen"
			);
			checkbox("Same Direction Angles", dynamicSyncPresetSettings.SameDirectionAngles, checkboxIndex++,
				"Applies to vertical and horizontal sync presets\n"
				"- Set all angles to the same direction"
			);
			Gui::Separator();
			checkbox("Inside Out Angles", dynamicSyncPresetSettings.InsideOutAngles, checkboxIndex++,
				"Applies to square and triangle sync presets\n"
				"- Flip angles by 180 degrees\n"
				"- Increase button distances"
			);
			checkbox("Elevate Bottom Row", dynamicSyncPresetSettings.ElevateBottomRow, checkboxIndex++,
				"Applies to square and triangle sync presets\n"
				"- Raise position height of bottom row targets by one 1/8th step"
			);

			Gui::EndPopup();
		}
	}

	void PresetWindow::SequenceGui(Chart& chart)
	{
#if COMFY_BEZIER_TEST
		BezierTestNew::DoGuiTest();
#endif

		hovered.Sequence.Preset = {};
		hovered.Sequence.AnyChildWindow = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		const auto& style = Gui::GetStyle();
		const bool anyTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected); });

		Gui::BeginChild("SequencePresetsChild", vec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			hovered.Sequence.ChildWindow = Gui::IsWindowHovered();

			const f32 fullWidth = Gui::GetContentRegionAvail().x;
			const f32 halfWidth = (fullWidth - PresetButtonSpacing.x) / 2.0f;

			Gui::PushItemDisabledAndTextColorIf(!anyTargetSelected);
			for (size_t i = 0; i < GlobalUserData.TargetPreset.SequencePresets.size(); i++)
			{
				const auto& thisSequencePreset = GlobalUserData.TargetPreset.SequencePresets[i];
				Gui::PushID(&thisSequencePreset);

				if (thisSequencePreset.ButtonType == SequencePresetButtonType::SingleLine)
				{
					if (Gui::ButtonEx(thisSequencePreset.Name.c_str(), vec2(fullWidth, SingleLineSequenceButtonHeight)))
						ApplySequencePresetToSelectedTargets(undoManager, chart, thisSequencePreset, sequencePresetSettings);
					if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						hovered.Sequence.Preset = i;
				}
				else if (thisSequencePreset.ButtonType == SequencePresetButtonType::SameLine)
				{
					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, PresetButtonSpacing);

					if (Gui::ButtonEx(thisSequencePreset.Name.c_str(), vec2(halfWidth, SameLineSequenceButtonHeight)))
						ApplySequencePresetToSelectedTargets(undoManager, chart, thisSequencePreset, sequencePresetSettings);
					if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						hovered.Sequence.Preset = i;

					if (i + 1 < GlobalUserData.TargetPreset.SequencePresets.size())
					{
						Gui::SameLine();

						const auto& nextSequencePreset = GlobalUserData.TargetPreset.SequencePresets[i + 1];
						Gui::PushID(&nextSequencePreset);

						if (Gui::ButtonEx(nextSequencePreset.Name.c_str(), vec2(halfWidth, SameLineSequenceButtonHeight)))
							ApplySequencePresetToSelectedTargets(undoManager, chart, nextSequencePreset, sequencePresetSettings);
						if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							hovered.Sequence.Preset = i + 1;

						Gui::PopID();
						++i;
					}

					Gui::PopStyleVar();

				}

				Gui::PopID();
			}
			Gui::PopItemDisabledAndTextColorIf(!anyTargetSelected);
		}
		Gui::EndChild();
	}

	void PresetWindow::UpdateStateAfterBothGuiPartsHaveBeenDrawn()
	{
		bool anyHovered = false;

		if (hovered.Sync.AnyChildWindow && !hovered.Sync.AddChildWindow && !hovered.Sync.ContextMenu)
			anyHovered = true;
		if (hovered.Sync.DynamicPreset.has_value() || hovered.Sync.StaticPreset.has_value())
			anyHovered = true;

		// TODO: Add checks for future additions
		if (hovered.Sequence.AnyChildWindow)
			anyHovered = true;
		if (hovered.Sequence.Preset.has_value())
			anyHovered = true;
		if (hovered.Sync.ContextMenuOpen)
			anyHovered = true;

		// HACK: Super hacky but mainly to prevent undesired hover detecting for cases like resize hovering a docked window...
		if (auto c = Gui::GetMouseCursor(); c != ImGuiMouseCursor_None && c != ImGuiMouseCursor_Arrow && c != ImGuiMouseCursor_Hand)
			anyHovered = false;

		// HACK: Again a bit hacky but should solve undesired fades while unfocused
		if (!chartEditor.GetParentApplication().GetHost().IsWindowFocused())
			anyHovered = false;

		hovered.AnyHoveredLastFrame = hovered.AnyHoveredThisFrame;
		hovered.AnyHoveredThisFrame = anyHovered;

		if (anyHovered)
			hovered.LastHoverStopwatch.Restart();
		else
			hovered.HoverDurationStopwatch.Restart();
	}

	void PresetWindow::OnRenderWindowRender(Chart& chart, TargetRenderWindow& renderWindow, Render::Renderer2D& renderer)
	{
		auto& renderHelper = renderWindow.GetRenderHelper();

#if COMFY_BEZIER_TEST
		BezierTestNew::DEBUG_RENDER_REGION = renderWindow.GetRenderRegion();

		for (const auto& position : BezierTestNew::CirclePositionsToPreview)
		{
			TargetRenderHelper::TargetData targetData = {};
			targetData.Type = ButtonType::Circle;
			targetData.Position = position;
			targetData.Scale = 1.0f;
			targetData.NoScale = true;
			targetData.Transparent = true;
			renderHelper.DrawTarget(renderer, targetData);
		}
#endif

		if (const auto dimness = GetPresetPreviewDimness(false); dimness > 0.0f)
			renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, dimness)));

		syncPresetPreview.TargetCount = 0;
		if (hovered.Sync.DynamicPreset.has_value())
		{
			syncPresetPreview.TargetCount = FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(chart, hovered.Sync.DynamicPreset.value(), dynamicSyncPresetSettings, syncPresetPreview.Targets);
			RenderSyncPresetPreview(renderer, renderHelper, syncPresetPreview.TargetCount, syncPresetPreview.Targets);
		}
		else if (hovered.Sync.StaticPreset.has_value())
		{
			const auto& hoveredPreset = GlobalUserData.TargetPreset.StaticSyncPresets[*hovered.Sync.StaticPreset];
			syncPresetPreview.TargetCount = hoveredPreset.TargetCount;
			syncPresetPreview.Targets = hoveredPreset.Targets;
			RenderSyncPresetPreview(renderer, renderHelper, hoveredPreset.TargetCount, hoveredPreset.Targets);
		}
		else if (hovered.Sequence.Preset.has_value())
		{
			// TODO: ...
			const auto& preset = GlobalUserData.TargetPreset.SequencePresets[*hovered.Sequence.Preset];

			if (preset.Type == SequencePresetType::Circle)
			{

			}
		}
	}

	void PresetWindow::OnRenderWindowOverlayGui(Chart& chart, TargetRenderWindow& renderWindow, ImDrawList& drawList)
	{
		const auto windowRect = Gui::GetCurrentWindow()->Rect();
		if (const auto dimness = GetPresetPreviewDimness(true); dimness > 0.0f)
			drawList.AddRectFilled(windowRect.GetTL(), windowRect.GetBR(), ImColor(0.0f, 0.0f, 0.0f, dimness));

		// NOTE: Helps to reduce visual noise while quickly mouse hover skipping over a preset window
		const f32 hoverFadeOpacity = GetHoverFadeInPreviewOpacity();
		const u32 hoverFadeOpacityU8 = static_cast<u8>(hoverFadeOpacity * std::numeric_limits<u8>::max());

		if (hovered.Sync.DynamicPreset.has_value() || hovered.Sync.StaticPreset.has_value())
		{
			for (u32 i = 0; i < syncPresetPreview.TargetCount; i++)
			{
				const auto& presetTarget = syncPresetPreview.Targets[i];

				const u32 color = GetButtonTypeColorU32(presetTarget.Type, hoverFadeOpacityU8);
				DrawCurvedButtonPathLine(renderWindow, drawList, presetTarget.Properties, color, 2.0f);

				const f32 stepDistance = 1.0f / static_cast<f32>(GlobalUserData.System.Gui.TargetButtonPathCurveSegments);

				const vec2 targetPos = GetButtonPathSinePoint(1.0f, presetTarget.Properties);
				const vec2 targetPosTangent = glm::normalize(GetButtonPathSinePoint(1.0f - stepDistance, presetTarget.Properties) - targetPos);
				DrawButtonPathArrowHead(renderWindow, drawList, targetPos, targetPosTangent, color, 2.0f);
			}
		}
		else if (hovered.Sequence.Preset.has_value())
		{
			// TODO: Proper implementation...
			const auto& preset = GlobalUserData.TargetPreset.SequencePresets[*hovered.Sequence.Preset];

			if (preset.Type == SequencePresetType::Circle)
			{
				const u32 circleColor = ImColor(0.69f, 0.69f, 0.69f, hoverFadeOpacity);
				drawList.AddCircle(renderWindow.TargetAreaToScreenSpace(preset.Circle.Center), preset.Circle.Radius * renderWindow.GetCamera().Zoom, circleColor, 64, 2.0f);

				constexpr i32 arrowCount = 8; // 4;
				for (i32 i = 0; i < arrowCount; i++)
				{
					const f32 angleRadians = static_cast<f32>(i) * (glm::two_pi<f32>() / static_cast<f32>(arrowCount)) * preset.Circle.Direction;

					const vec2 normal = vec2(glm::cos(angleRadians), glm::sin(angleRadians));
					const vec2 tangent = vec2(normal.y, -normal.x) * preset.Circle.Direction;
					const vec2 pointOnCircle = preset.Circle.Center + normal * preset.Circle.Radius;

					DrawButtonPathArrowHeadCentered(renderWindow, drawList, pointOnCircle, tangent, circleColor, 2.0f);
				}
			}
		}
	}

	void PresetWindow::OnEditorSpritesLoaded(const Graphics::SprSet* sprSet)
	{
		editorSprites = sprSet;
		if (editorSprites == nullptr)
			return;

		auto findSprite = [this](std::string_view spriteName)
		{
			const auto found = FindIfOrNull(editorSprites->Sprites, [&](const auto& spr) { return spr.Name == spriteName; });
			return (found != nullptr && InBounds(found->TextureIndex, editorSprites->TexSet.Textures)) ? found : nullptr;
		};

		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::VerticalLeft)] = findSprite("SYNC_PRESET_ICON_VERTICAL_LEFT");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::VerticalRight)] = findSprite("SYNC_PRESET_ICON_VERTICAL_RIGHT");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::HorizontalUp)] = findSprite("SYNC_PRESET_ICON_HORIZONTAL_UP");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::HorizontalDown)] = findSprite("SYNC_PRESET_ICON_HORIZONTAL_DOWN");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::Square)] = findSprite("SYNC_PRESET_ICON_SQUARE");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::Triangle)] = findSprite("SYNC_PRESET_ICON_TRIANGLE");
	}

	f32 PresetWindow::GetPresetPreviewDimness(bool overlayPass) const
	{
		const f32 max = overlayPass ? MaxDimOverlay : MaxDimRender;

		if (hovered.AnyHoveredThisFrame)
		{
			if (!hovered.HoverDurationStopwatch.IsRunning())
				return max;

			const f32 hoverMS = static_cast<f32>(hovered.HoverDurationStopwatch.GetElapsed().TotalMilliseconds());
			return Clamp(ConvertRange<f32>(0.0f, HoverFadeInMS, 0.0f, max, hoverMS), 0.0f, max);
		}
		else
		{
			if (!hovered.LastHoverStopwatch.IsRunning())
				return 0.0f;

			const f32 sinceHoverMS = static_cast<f32>(hovered.LastHoverStopwatch.GetElapsed().TotalMilliseconds());
			return Clamp(ConvertRange<f32>(0.0f, HoverFadeOutMS, max, 0.0f, sinceHoverMS), 0.0f, max);
		}
	}

	f32 PresetWindow::GetHoverFadeInPreviewOpacity() const
	{
		const f32 hoverMS = static_cast<f32>(hovered.HoverDurationStopwatch.GetElapsed().TotalMilliseconds());
		const f32 opacity = Clamp<f32>(ConvertRange<f32>(0.0f, HoverFadeInMS, 0.0f, 1.0f, hoverMS), 0.0f, 1.0f);
		return (opacity * opacity);
	}

	void PresetWindow::RenderSyncPresetPreview(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, u32 targetCount, const std::array<PresetTargetData, Rules::MaxSyncPairCount>& presetTargets)
	{
		assert(targetCount <= presetTargets.size());
		if (targetCount < 1)
			return;

		const f32 hoverFadeOpacity = GetHoverFadeInPreviewOpacity();

		TargetRenderHelper::ButtonSyncLineData syncLineData = {};
		TargetRenderHelper::TargetData targetData = {};

#if 0 // NOTE: Not rendering these as sync targets improves readability when rendering on top of the existing targets to be replaced
		targetData.Sync = (targetCount > 1);
#endif

		syncLineData.SyncPairCount = targetCount;
		syncLineData.Progress = 0.0f;
		syncLineData.Scale = 1.0f;
		syncLineData.Opacity = hoverFadeOpacity;
		for (size_t i = 0; i < targetCount; i++)
		{
			syncLineData.TargetPositions[i] = presetTargets[i].Properties.Position;
			syncLineData.ButtonPositions[i] = presetTargets[i].Properties.Position;
		}
		renderHelper.DrawButtonPairSyncLines(renderer, syncLineData);

		for (size_t i = 0; i < targetCount; i++)
		{
			const auto& presetTarget = presetTargets[i];

			targetData.Type = presetTarget.Type;
			targetData.Position = presetTarget.Properties.Position;
			targetData.NoScale = true;
			targetData.Scale = 1.0f;
			targetData.Opacity = hoverFadeOpacity * 0.5f;
			renderHelper.DrawTarget(renderer, targetData);
		}
	}
}
