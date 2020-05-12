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
		RenderCommand2D() = default;

	public:
		const Graphics::Tex* Texture = nullptr;
		vec2 Origin = { 0.0f, 0.0f };
		vec2 Position = { 0.0f, 0.0f };
		float Rotation = 0.0f;
		vec2 Scale = { 1.0f, 1.0f };
		vec4 SourceRegion;
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Normal;
		std::array<vec4, 4> CornerColors = { vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) };

		// TODO: Extra flags, texture filter, wrap, etc.
		// bool DrawTextBorder = false;
	};

	class Renderer2D : NonCopyable
	{
	public:
		static constexpr u32 MaxBatchItemSize = 2048;

	public:
		Renderer2D();
		~Renderer2D() = default;

	public:
		void Begin(const OrthographicCamera& camera);
		void Draw(const RenderCommand2D& command);
		void Draw(const RenderCommand2D& command, const RenderCommand2D& commandMask);

		/*
		void Draw(vec2 position, vec2 size, vec4 color);
		void Draw(vec2 position, vec2 size, const vec4 colors[4]);
		void Draw(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color);

		void Draw(const Texture2D* texture, vec2 position, vec4 color);
		void Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec4 color);

		void Draw(const Texture2D* texture, vec2 position, vec2 origin, float rotation, vec4 color);
		void Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color, AetBlendMode blendMode = AetBlendMode::Normal);

		void Draw(
			const Texture2D* maskTexture, vec4 maskSourceRegion, vec2 maskPosition, vec2 maskOrigin, float maskRotation, vec2 maskScale,
			const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color,
			AetBlendMode blendMode);
		*/

		// TODO:
		void DrawLine(vec2 start, vec2 end, vec4 color, float thickness = 1.0f);
		void DrawLine(vec2 start, float angle, float length, vec4 color, float thickness = 1.0f);

		void DrawRect(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, vec4 color, float thickness = 1.0f);
		void DrawRectCheckerboard(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color, float precision = 1.0f);

		void End();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
