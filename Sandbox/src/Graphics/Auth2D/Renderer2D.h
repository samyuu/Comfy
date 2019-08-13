#pragma once
#include "Graphics/IRenderer.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/ComfyVertex.h"
#include "Graphics/Texture.h"
#include "Graphics/Camera.h"
#include "FileSystem/Format/AetSet.h"

namespace Auth2D
{
	using namespace FileSystem;

	struct SpriteIndices
	{
		uint16_t TopLeft;
		uint16_t BottomLeft;
		uint16_t BottomRight;
		uint16_t BottomRightCopy;
		uint16_t TopRight;
		uint16_t TopLeftCopy;

	public:
		static inline constexpr uint32_t GetIndexCount() { return sizeof(SpriteIndices) / sizeof(uint16_t); };
	};

	struct SpriteVertices
	{
		SpriteVertex TopLeft;
		SpriteVertex TopRight;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

	public:
		void SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color);
		void SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4 colors[4]);
		static inline constexpr uint32_t GetVertexCount() { return sizeof(SpriteVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositions(const vec2& position, const vec2& size);
		void SetPositions(const vec2& position, const vec2& size, const vec2& origin, float rotation);
		void SetTexCoords(const vec2& topLeft, const vec2& bottomRight);
		void SetTexMaskCoords(const vec2& topLeft, const vec2& bottomRight);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);
	};

	struct Batch
	{
		uint16_t Index;
		uint16_t Count;

		Batch(uint16_t index, uint16_t count) : Index(index), Count(count) {};
	};

	struct BatchItem
	{
		const Texture2D* Texture;
		const Texture2D* MaskTexture;
		AetBlendMode BlendMode;
		vec2 CheckerboardSize;

		void SetValues(const Texture2D* texture, const Texture2D* alphaMask = nullptr, AetBlendMode blendMode = AetBlendMode::Alpha);
	};

	struct BatchPair
	{
		BatchItem* Item;
		SpriteVertices* Vertices;
	};

	struct BlendFuncStruct
	{
		GLenum SourceRGB, DestinationRGB;
		GLenum SourceAlpha, DestinationAlpha;
	};

	constexpr uint32_t Renderer2DMaxItemSize = 2048;

	class Renderer2D : public IRenderer
	{
	public:
		void Initialize();
		void Begin(const OrthographicCamera& camera);
		void Draw(const vec2& position, const vec2& size, const vec4& color);
		void Draw(const vec2& position, const vec2& size, const vec4 colors[4]);
		void Draw(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color);

		void Draw(const Texture2D* texture, const vec2& position, const vec4& color);
		void Draw(const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec4& color);

		void Draw(const Texture2D* texture, const vec2& position, const vec2& origin, float rotation, const vec4& color);
		void Draw(const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color, AetBlendMode blendMode = AetBlendMode::Alpha);
		
		void Draw(
			const Texture2D* maskTexture, const vec4& maskSourceRegion, const vec2& maskPosition, const vec2& maskOrigin, float maskRotation, const vec2& maskScale,
			const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color,
			AetBlendMode blendMode);
		
		void DrawLine(const vec2& start, const vec2& end, const vec4& color, float thickness = 1.0f);
		void DrawLine(const vec2& start, float angle, float length, const vec4& color, float thickness = 1.0f);
		
		void DrawRectangle(const vec2& topLeft, const vec2& topRight, const vec2& bottomLeft, const vec2& bottomRight, const vec4& color, float thickness = 1.0f);
		void DrawCheckerboardRectangle(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color, float precision = 1.0f);

		const SpriteVertices& GetLastVertices() const;

		void End();
		void Flush();

		inline SpriteShader* GetShader() { return shader.get(); };
		inline const OrthographicCamera* GetCamera() const { return camera; };

		inline bool GetEnableAlphaTest() const { return enableAlphaTest; };
		inline void SetEnableAlphaTest(bool value) { enableAlphaTest = value; };

		inline bool GetUseTextShadow() const { return useTextShadow; };
		inline void SetUseTextShadow(bool value) { useTextShadow = value; };

		inline uint16_t GetDrawCallCount() const { return drawCallCount; };

		static void SetBlendFunction(AetBlendMode blendMode);
		static const BlendFuncStruct GetBlendFuncParamteres(AetBlendMode blendMode);

	private:
		bool enableAlphaTest = true;
		bool useTextShadow = false;

		UniquePtr<SpriteShader> shader;

		VertexArray vertexArray;
		VertexBuffer vertexBuffer = { BufferUsage::StreamDraw };
		IndexBuffer indexBuffer = { BufferUsage::StreamDraw, IndexType::UnsignedShort };

		bool batchSprites = true;
		std::vector<Batch> batches;
		std::vector<BatchItem> batchItems;
		std::vector<SpriteVertices> vertices;

		const OrthographicCamera* camera;

		uint16_t drawCallCount;

		void GenerateUploadSpriteIndexBuffer(uint16_t elementCount);
		void CreateBatches();
		
		inline void CheckFlushItems();
		inline BatchPair CheckFlushAddItem();
		inline BatchPair AddItem();
		void ClearItems();
		
		void DrawInternal(const Texture2D* texture, const vec4* sourceRegion, const vec2* position, const vec2* origin, float rotation, const vec2* scale, const vec4* color, AetBlendMode blendMode = AetBlendMode::Alpha);
	};
}
