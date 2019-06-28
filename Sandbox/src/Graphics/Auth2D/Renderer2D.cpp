#include "Renderer2D.h"

namespace Auth2D
{
	void RectangleVertices::SetValues(const vec2& position, const vec2& size, const vec4& color)
	{
		SetPositions(position, size);
		SetTexCoords(vec2(0, 1), vec2(1, 0));
		SetColors(color);

		BottomRightCopy = BottomRight;
		TopLeftCopy = TopLeft;
	}

	void RectangleVertices::SetPositions(const vec2& position, const vec2& size)
	{
		TopLeft.Position.x = position.x;
		TopLeft.Position.y = position.y;

		TopRight.Position.x = position.x + size.x;
		TopRight.Position.y = position.y;

		BottomLeft.Position.x = position.x;
		BottomLeft.Position.y = position.y + size.y;

		BottomRight.Position.x = position.x + size.x;
		BottomRight.Position.y = position.y + size.y;
	}

	void RectangleVertices::SetTexCoords(const vec2& topLeft, const vec2& bottomRight)
	{
		TopLeft.TextureCoordinates.x = topLeft.x;
		TopLeft.TextureCoordinates.y = topLeft.y;

		TopRight.TextureCoordinates.x = bottomRight.x;
		TopRight.TextureCoordinates.y = topLeft.y;

		BottomLeft.TextureCoordinates.x = topLeft.x;
		BottomLeft.TextureCoordinates.y = bottomRight.y;

		BottomRight.TextureCoordinates.x = bottomRight.x;
		BottomRight.TextureCoordinates.y = bottomRight.y;
	}

	void RectangleVertices::SetColors(const vec4& color)
	{
		TopLeft.Color = color;
		TopRight.Color = color;
		BottomLeft.Color = color;
		BottomRight.Color = color;
	}

	void BatchItem::SetValues(const Texture2D* texture, const Texture2D* alphaMask, AetBlendMode blendMode)
	{
		Texture = texture;
		MaskTexture = alphaMask;
		BlendMode = blendMode;
	}

	void Renderer2D::Initialize()
	{
		BufferLayout layout =
		{
			{ ShaderDataType::Vec2, "in_position" },
			{ ShaderDataType::Vec2, "in_texture_coords" },
			{ ShaderDataType::Vec4, "in_color" }
		};

		vertexBuffer.InitializeID();
		vertexBuffer.Bind();

		shader = std::make_unique<SpriteShader>();
		shader->Initialize();
		shader->Bind();

		vertexArray.InitializeID();
		vertexArray.Bind();
		vertexArray.SetLayout(layout);

		constexpr size_t initialCapacity = 64;
		batchItems.reserve(initialCapacity);
		vertices.reserve(initialCapacity);
	}

	void Renderer2D::Begin()
	{
	}

	void Renderer2D::Draw(const vec2& position, const vec2 size, const vec4& color)
	{
		BatchPair pair = AddItem();

		pair.Item->SetValues(nullptr);
		pair.Vertices->SetValues(position, size, color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec2& position, const vec4& color, AetBlendMode blendMode)
	{
		BatchPair pair = AddItem();

		pair.Item->SetValues(texture, nullptr, blendMode);
		pair.Vertices->SetValues(position, texture->GetSize(), color);
	}

	void Renderer2D::End()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		glEnable(GL_BLEND);

		vertexArray.Bind();
		vertexBuffer.Bind();
		vertexBuffer.UploadSubData(vertices.size() * sizeof(RectangleVertices), vertices.data());
		shader->Bind();
		
		for (size_t i = 0; i < batchItems.size(); i++)
		{
			BatchItem& item = batchItems[i];

			SetBlendFunction(item.BlendMode);

			if (item.Texture != nullptr)
			{
				item.Texture->Bind();
				shader->SetUniform(shader->TextureFormatLocation, (int)item.Texture->GetTextureFormat());
			}

			shader->SetUniform(shader->UseSolidColorLocation, item.Texture == nullptr);

			constexpr auto elementSize = sizeof(RectangleVertices) / sizeof(SpriteVertex);
			glDrawArrays(GL_TRIANGLES, i * elementSize, elementSize);
		}

		ClearItems();
	}

	void Renderer2D::Resize(float width, float height)
	{
		mat4 projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

		shader->Bind();
		shader->SetUniform(shader->ProjectionLocation, projection);
	}

	void Renderer2D::SetBlendFunction(AetBlendMode blendMode)
	{
		const BlendFuncStruct& blendFunc = GetBlendFuncParamteres(blendMode);
		glBlendFuncSeparate(
			blendFunc.SourceRGB,
			blendFunc.DestinationRGB,
			blendFunc.SourceAlpha, 
			blendFunc.DestinationAlpha);
	}

	const BlendFuncStruct& Renderer2D::GetBlendFuncParamteres(AetBlendMode blendMode)
	{
		static BlendFuncStruct lookupTable[6] =
		{
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE },
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE },
			{ GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE },
			{ GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE },
			{ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO },
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE },
		};

		int lookupIndex = 0;
		switch (blendMode)
		{
		case AetBlendMode::Alpha: 
			lookupIndex = 0; break;
		case AetBlendMode::Additive: 
			lookupIndex = 2; break;
		case AetBlendMode::DstColorZero: 
			lookupIndex = 3; break;
		case AetBlendMode::SrcAlphaOneMinusSrcColor: 
			lookupIndex = 1; break;
		case AetBlendMode::Transparent: 
			lookupIndex = 5; break;
		}
		return lookupTable[lookupIndex];
	}

	BatchPair Renderer2D::AddItem()
	{
		batchItems.emplace_back();
		vertices.emplace_back();

		return { &batchItems.back(), &vertices.back() };
	}

	void Renderer2D::ClearItems()
	{
		batchItems.clear();
		vertices.clear();
	}
}
