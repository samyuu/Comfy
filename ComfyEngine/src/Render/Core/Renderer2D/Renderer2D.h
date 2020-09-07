#pragma once
#include "Types.h"
#include "RenderCommand2D.h"
#include "RenderTarget2D.h"
#include "AetRenderer.h"
#include "FontRenderer.h"
#include "Render/Core/Camera.h"
#include "Resource/IDTypes.h"
#include <optional>

namespace Comfy::Render
{
	struct PositionTextureColorVertex
	{
		vec2 Position;
		vec2 TextureCoordinates;
		vec4 Color;
	};

	class Renderer2D : NonCopyable
	{
	public:
		Renderer2D();
		~Renderer2D();

	public:
		void Begin(Camera2D& camera, RenderTarget2D& renderTarget);
		void Draw(const RenderCommand2D& command);
		void Draw(const RenderCommand2D& command, const RenderCommand2D& commandMask);

		void DrawLine(vec2 start, vec2 end, const vec4& color, float thickness = 1.0f);
		void DrawLine(vec2 start, float angle, float length, const vec4& color, float thickness = 1.0f);

		void DrawRect(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, const vec4& color, float thickness = 1.0f);
		void DrawRectCheckerboard(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4& color, float precision = 1.0f);

		void DrawVertices(const PositionTextureColorVertex* vertices, size_t vertexCount,
			TexSamplerView texView = nullptr,
			Graphics::AetBlendMode blendMode = Graphics::AetBlendMode::Normal,
			Graphics::PrimitiveType primitive = Graphics::PrimitiveType::Triangles);

		void End();

	public:
		AetRenderer& Aet();
		FontRenderer& Font();

	public:
		// NOTE: Optionally pre upload all resources that are known to be static instead of doing it just in time to reduce memory usage
		void UploadToGPUFreeCPUMemory(Graphics::SprSet& sprSet);
		void UploadToGPUFreeCPUMemory(Graphics::TexSet& texSet);
		void UploadToGPUFreeCPUMemory(Graphics::Tex& tex);
	
	public:
		static std::unique_ptr<RenderTarget2D> CreateRenderTarget();

	public:
		// NOTE: Only valid between a Begin() / End() call
		const Camera2D& GetCamera() const;
		RenderTarget2D& GetRenderTarget() const;

		/*
		const SpriteVertices& GetLastVertices() const;

		bool GetDrawTextBorder() const;
		void SetDrawTextBorder(bool value);

		u32 GetDrawCallCount() const;
		*/

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
