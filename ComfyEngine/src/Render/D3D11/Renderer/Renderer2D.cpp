#include "Renderer2D.h"
#include "../Shader/Bytecode/ShaderBytecode.h"

namespace Comfy::Render::D3D11
{
	void Renderer2D::Begin(const OrthographicCamera& camera)
	{

	}

	void Renderer2D::Draw(vec2 position, vec2 size, vec4 color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		InternalDraw(nullptr, &source, &position, nullptr, DefaultProperties::Rotation, nullptr, &color);
	}

	void Renderer2D::Draw(vec2 position, vec2 size, const vec4 colors[4])
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);

		SpriteBatchPair pair = InternalCheckFlushAddItem();
		pair.Item->SetValues(nullptr, nullptr);
		pair.Vertices->SetValues(position, source, size, DefaultProperties::Origin, DefaultProperties::Rotation, DefaultProperties::Scale, colors);
	}

	void Renderer2D::Draw(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		InternalDraw(nullptr, &source, &position, &origin, rotation, &scale, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, vec2 position, vec4 color)
	{
		InternalDraw(texture, nullptr, &position, nullptr, DefaultProperties::Rotation, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec4 color)
	{
		InternalDraw(texture, &sourceRegion, &position, nullptr, 0.0f, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, vec2 position, vec2 origin, float rotation, vec4 color)
	{
		InternalDraw(texture, nullptr, &position, &origin, rotation, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color, AetBlendMode blendMode)
	{
		InternalDraw(texture, &sourceRegion, &position, &origin, rotation, &scale, &color, blendMode);
	}

	void Renderer2D::Draw(
		const Texture2D* maskTexture, vec4 maskSourceRegion, vec2 maskPosition, vec2 maskOrigin, float maskRotation, vec2 maskScale,
		const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color,
		AetBlendMode blendMode)
	{

	}

	void Renderer2D::DrawLine(vec2 start, vec2 end, vec4 color, float thickness)
	{

	}

	void Renderer2D::DrawLine(vec2 start, float angle, float length, vec4 color, float thickness)
	{

	}

	void Renderer2D::DrawRectangle(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, vec4 color, float thickness)
	{

	}

	void Renderer2D::DrawCheckerboardRectangle(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color, float precision)
	{

	}

	const SpriteVertices& Renderer2D::GetLastVertices() const
	{
		return vertices.back();
	}

	const OrthographicCamera* Renderer2D::GetCamera() const
	{
		return orthographicCamera;
	}

	bool Renderer2D::GetDrawTextBorder() const
	{
		return drawTextBorder;
	}

	void Renderer2D::SetDrawTextBorder(bool value)
	{
		drawTextBorder = value;
	}

	u32 Renderer2D::GetDrawCallCount() const
	{
		return drawCallCount;
	}
}
