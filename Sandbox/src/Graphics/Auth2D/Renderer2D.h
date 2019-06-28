#pragma once
#include "Graphics/IRenderer.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/ComfyVertex.h"
#include "Graphics/Texture.h"
#include "FileSystem/Format/AetSet.h"

using namespace FileSystem;

namespace Auth2D
{
	struct RectangleVertices
	{
		//SpriteVertex TopLeft;
		//SpriteVertex TopRight;
		//SpriteVertex BottomLeft;
		//SpriteVertex BottomRight;

		SpriteVertex TopLeft;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

		SpriteVertex BottomRightCopy;
		SpriteVertex TopRight;
		SpriteVertex TopLeftCopy;

	public:
		void SetValues(const vec2& position, const vec2& size, const vec4& color);
	private:
		void SetPositions(const vec2& position, const vec2& size);
		void SetTexCoords(const vec2& topLeft, const vec2& bottomRight);
		void SetColors(const vec4& color);
	};

	struct BatchItem
	{
		const Texture2D* Texture;
		const Texture2D* MaskTexture;
		AetBlendMode BlendMode;

		void SetValues(const Texture2D* texture, const Texture2D* alphaMask = nullptr, AetBlendMode blendMode = AetBlendMode::Alpha);
	};

	struct BatchPair
	{
		BatchItem* Item;
		RectangleVertices* Vertices;
	};

	struct BlendFuncStruct
	{
		GLenum SourceRGB, DestinationRGB;
		GLenum SourceAlpha, DestinationAlpha;
	};

	class Renderer2D : public IRenderer
	{
	public:
		void Initialize();
		void Begin();
		void Draw(const vec2& position, const vec2 size, const vec4& color);
		void Draw(const Texture2D* texture, const vec2& position, const vec4& color, AetBlendMode blendMode = AetBlendMode::Alpha);
		void End();
		void Flush();

		void Resize(float width, float height);
		static void SetBlendFunction(AetBlendMode blendMode);
		static const BlendFuncStruct& GetBlendFuncParamteres(AetBlendMode blendMode);

	private:
		std::unique_ptr<SpriteShader> shader;
		VertexBuffer vertexBuffer = { BufferUsage::DynamicDraw };
		VertexArray vertexArray;

		std::vector<BatchItem> batchItems;
		std::vector<RectangleVertices> vertices;

		BatchPair AddItem();
		void ClearItems();
	};
}
