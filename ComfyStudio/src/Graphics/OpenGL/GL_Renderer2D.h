#pragma once
#include "GL_Buffer.h"
#include "GL_VertexArray.h"
#include "GL_Shaders.h"
#include "GL_Texture2D.h"
#include "Graphics/Auth2D/SpriteBatchData.h"
#include "Graphics/Camera.h"

namespace Graphics
{
	struct GL_BlendFuncStruct
	{
		GLenum SourceRGB, DestinationRGB;
		GLenum SourceAlpha, DestinationAlpha;
	};

	class GL_Renderer2D /* : public IRenderer */
	{
	public:
		static constexpr uint32_t MaxBatchItemSize = 2048;

	public:
		void Initialize();
		void Begin(const OrthographicCamera& camera);
		void Draw(const vec2& position, const vec2& size, const vec4& color);
		void Draw(const vec2& position, const vec2& size, const vec4 colors[4]);
		void Draw(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color);

		void Draw(const GL_Texture2D* texture, const vec2& position, const vec4& color);
		void Draw(const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec4& color);

		void Draw(const GL_Texture2D* texture, const vec2& position, const vec2& origin, float rotation, const vec4& color);
		void Draw(const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color, AetBlendMode blendMode = AetBlendMode::Normal);
		
		void Draw(
			const GL_Texture2D* maskTexture, const vec4& maskSourceRegion, const vec2& maskPosition, const vec2& maskOrigin, float maskRotation, const vec2& maskScale,
			const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color,
			AetBlendMode blendMode);
		
		void DrawLine(const vec2& start, const vec2& end, const vec4& color, float thickness = 1.0f);
		void DrawLine(const vec2& start, float angle, float length, const vec4& color, float thickness = 1.0f);
		
		void DrawRectangle(const vec2& topLeft, const vec2& topRight, const vec2& bottomLeft, const vec2& bottomRight, const vec4& color, float thickness = 1.0f);
		void DrawCheckerboardRectangle(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color, float precision = 1.0f);

		void End();

		const SpriteVertices& GetLastVertices() const;

		inline GL_SpriteShader* GetSpriteShader() { return spriteShader.get(); };
		inline const OrthographicCamera* GetCamera() const { return orthographicCamera; };

		inline bool GetEnableAlphaTest() const { return enableAlphaTest; };
		inline void SetEnableAlphaTest(bool value) { enableAlphaTest = value; };

		inline bool GetUseTextShadow() const { return useTextShadow; };
		inline void SetUseTextShadow(bool value) { useTextShadow = value; };

		inline uint16_t GetDrawCallCount() const { return drawCallCount; };

		static void SetBlendFunction(AetBlendMode blendMode);
		static const GL_BlendFuncStruct GetBlendFuncParamteres(AetBlendMode blendMode);

	private:
		bool enableAlphaTest = true;
		bool useTextShadow = false;

		UniquePtr<GL_SpriteShader> spriteShader = nullptr;

		GL_VertexArray vertexArray = {};
		GL_VertexBuffer vertexBuffer = { BufferUsage::StreamDraw };
		GL_IndexBuffer indexBuffer = { BufferUsage::StreamDraw, IndexType::UInt16 };

		bool batchSprites = true;
		std::vector<SpriteBatch> batches;
		std::vector<SpriteBatchItem> batchItems;
		std::vector<SpriteVertices> vertices;

		const OrthographicCamera* orthographicCamera = nullptr;

		uint16_t drawCallCount = 0;

		void GenerateUploadSpriteIndexBuffer();
		void CreateBatches();
		
		void Flush();
		void CheckFlushItems();
		SpriteBatchPair CheckFlushAddItem();
		SpriteBatchPair AddItem();
		void ClearItems();
		
		void DrawInternal(const GL_Texture2D* texture, const vec4* sourceRegion, const vec2* position, const vec2* origin, float rotation, const vec2* scale, const vec4* color, AetBlendMode blendMode = AetBlendMode::Normal);
	};
}
