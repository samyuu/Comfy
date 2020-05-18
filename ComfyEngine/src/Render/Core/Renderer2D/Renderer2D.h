#pragma once
#include "Types.h"
#include "Render/Core/Camera.h"
#include "Resource/IDTypes.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include <optional>

namespace Comfy::Render
{
	struct RenderCommand2D
	{
	public:
		// TODO: Various constructors
		RenderCommand2D() = default;

		RenderCommand2D(const Graphics::Tex* texture, vec2 origin, vec2 position, float rotation, vec2 scale, vec4 sourceRegion, Graphics::AetBlendMode blendMode, float opacity)
			: Texture(texture), Origin(origin), Position(position), Rotation(rotation), Scale(scale), SourceRegion(sourceRegion), BlendMode(blendMode)
		{
			for (auto& color : CornerColors)
				color.a = opacity;
		}

		void SetColor(const vec4& newColor)
		{
			for (auto& color : CornerColors)
				color = newColor;
		}

	public:
		// NOTE: Can be null to draw a solid color rectangle instead
		const Graphics::Tex* Texture = nullptr;

		vec2 Origin = { 0.0f, 0.0f };
		vec2 Position = { 0.0f, 0.0f };
		float Rotation = 0.0f;
		vec2 Scale = { 1.0f, 1.0f };
		vec4 SourceRegion = { 0.0f, 0.0f, 1.0f, 1.0f };
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Normal;

		// NOTE: Top left, top right, bottom left, bottom right
		std::array<vec4, 4> CornerColors = { vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) };

		// TODO: Extra flags, texture filter, wrap (problematic with atlases), etc.
		bool DrawTextBorder = false;
	};

	class Renderer2D : NonCopyable
	{
	public:
		static constexpr u32 MaxBatchItemSize = 2048;

	public:
		Renderer2D();
		~Renderer2D();

	public:
		void Begin(OrthographicCamera& camera);
		void Draw(const RenderCommand2D& command);
		void Draw(const RenderCommand2D& command, const RenderCommand2D& commandMask);

		void DrawLine(vec2 start, vec2 end, const vec4& color, float thickness = 1.0f);
		void DrawLine(vec2 start, float angle, float length, const vec4& color, float thickness = 1.0f);

		void DrawRect(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, const vec4& color, float thickness = 1.0f);
		void DrawRectCheckerboard(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4& color, float precision = 1.0f);

		void End();

		/*
	public:
		const SpriteVertices& GetLastVertices() const;
		const OrthographicCamera* GetCamera() const;

		bool GetDrawTextBorder() const;
		void SetDrawTextBorder(bool value);

		u32 GetDrawCallCount() const;
		*/

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
