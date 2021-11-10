#pragma once
#include "Types.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	constexpr vec4 GridColor = vec4(0.527f, 0.527f, 0.527f, 0.737f);
	constexpr vec4 GridColorSub = vec4(0.586f, 0.586f, 0.586f, 0.255f);

	constexpr ivec2 GridLineCount = ivec2(19, 8);
	constexpr i32 GridSubDivisions = 8;

	constexpr i32 GridVertexCountRough = (GridLineCount.x + GridLineCount.y) * 2;
	constexpr i32 GridVertexCountSub = (((GridLineCount.x - 1) * GridSubDivisions + 1) + ((GridLineCount.y - 1) * GridSubDivisions + 1)) * 2;
	constexpr i32 GridVertexCount = GridVertexCountRough + GridVertexCountSub;

	static_assert(GridVertexCountRough == 54);
	static_assert(GridVertexCountSub == 404);
	static_assert(GridVertexCount == 458);

	constexpr std::array<Render::PositionTextureColorVertex, GridVertexCount> GenerateGridVertices()
	{
		std::array<Render::PositionTextureColorVertex, GridVertexCount> vertices = {};

		for (size_t i = 0; i < GridVertexCount; i++)
			vertices[i].TextureCoordinates = vec2(0.5f);

		for (size_t i = 0; i < GridVertexCountRough; i++)
			vertices[i].Color = GridColor;

		for (size_t i = GridVertexCountRough; i < GridVertexCount; i++)
			vertices[i].Color = GridColorSub;

		constexpr vec2 min = Rules::RecommendedPlacementAreaMin, max = Rules::RecommendedPlacementAreaMax;
		constexpr f32 gridStep = Rules::TickToDistance((BeatTick::FromBars(1) / 8));
		constexpr f32 gridStepSub = Rules::TickToDistance((BeatTick::FromBars(1) / 8)) / static_cast<f32>(GridSubDivisions);

		size_t v = 0;
		for (i32 x = 0; x < GridLineCount.x; x++)
		{
			vertices[v++].Position = min + vec2(gridStep * x, 0.0f);
			vertices[v++].Position = min + vec2(gridStep * x, max.y - min.y);
		}
		for (i32 y = 0; y < GridLineCount.y; y++)
		{
			vertices[v++].Position = min + vec2(0.0f, gridStep * y);
			vertices[v++].Position = min + vec2(max.x - min.x, gridStep * y);
		}

		for (i32 x = 0; x <= (GridLineCount.x - 1) * GridSubDivisions; x++)
		{
			vertices[v++].Position = min + vec2(gridStepSub * x, 0.0f);
			vertices[v++].Position = min + vec2(gridStepSub * x, max.y - min.y);
		}
		for (i32 y = 0; y <= (GridLineCount.y - 1) * GridSubDivisions; y++)
		{
			vertices[v++].Position = min + vec2(0.0f, gridStepSub * y);
			vertices[v++].Position = min + vec2(max.x - min.x, gridStepSub * y);
		}

		return vertices;
	}

	constexpr auto GridVertices = GenerateGridVertices();

	inline void RenderTargetGrid(Render::Renderer2D& renderer, bool drawHorizontalSyncMarkers)
	{
		renderer.DrawVertices(GridVertices.data(), GridVertices.size(), nullptr, Graphics::AetBlendMode::Normal, Graphics::PrimitiveType::Lines);

		if (drawHorizontalSyncMarkers)
		{
			constexpr f32 gridStep = Rules::TickToDistance((BeatTick::FromBars(1) / 8));
			constexpr f32 markerY = Rules::RecommendedPlacementAreaMin.y + (gridStep * 4.0f);

			Render::RenderCommand2D renderCommand = {};
			renderCommand.SetColor(GridColor);
			renderCommand.Origin = vec2(0.5f, 0.5f);
			renderCommand.Scale = vec2(2.0f / renderer.GetCamera().Zoom, Rules::GridStepDistance * 1.5f);
			renderCommand.Position.y = markerY;

			for (size_t i = 0; i < Rules::HorizontalSyncPairPositionsX.size(); i++)
			{
				renderCommand.Position.x = Rules::HorizontalSyncPairPositionsX[i];

				renderCommand.Rotation = 0.0f;
				renderer.Draw(renderCommand);
				renderCommand.Rotation = 90.0f;
				renderer.Draw(renderCommand);
			}
		}
	}
}
