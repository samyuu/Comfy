#pragma once
#include "Types.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	constexpr auto GridColor = vec4(0.527f, 0.527f, 0.527f, 0.737f);
	constexpr auto GridColorSub = vec4(0.586f, 0.586f, 0.586f, 0.255f);

	constexpr auto GridLineCount = ivec2(19, 8);
	constexpr auto GridSubDivisions = 8;

	constexpr auto GridVertexCountRough = (GridLineCount.x + GridLineCount.y) * 2;
	constexpr auto GridVertexCountSub = (((GridLineCount.x - 1) * GridSubDivisions + 1) + ((GridLineCount.y - 1) * GridSubDivisions + 1)) * 2;
	constexpr auto GridVertexCount = GridVertexCountRough + GridVertexCountSub;

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

		constexpr auto min = Rules::RecommendedPlacementAreaMin, max = Rules::RecommendedPlacementAreaMax;
		constexpr auto gridStep = Rules::TickToDistance((TimelineTick::FromBars(1) / 8));
		constexpr auto gridStepSub = Rules::TickToDistance((TimelineTick::FromBars(1) / 8)) / static_cast<f32>(GridSubDivisions);

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

	inline void RenderTargetGrid(Render::Renderer2D& renderer)
	{
		renderer.DrawVertices(GridVertices.data(), GridVertices.size(), nullptr, Graphics::AetBlendMode::Normal, Graphics::PrimitiveType::Lines);
	}
}
