#pragma once
#include "Types.h"

namespace Graphics
{
	typedef int32_t ClearTarget;
	enum ClearTarget_Enum : ClearTarget
	{
		ClearTarget_None = 0,
		ClearTarget_ColorBuffer = 1 << 0,
		ClearTarget_DepthBuffer = 1 << 1,
		ClearTarget_AccumBuffer = 1 << 2,
		ClearTarget_StencilBuffer = 1 << 3,
	};

	enum class PrimitiveType : uint32_t
	{
		Points,
		Lines,
		Triangles = 4,
		TriangleStrip = 5,
	};

	enum TextureSlot : int32_t
	{
		TextureSlot_0,
		TextureSlot_1,
		TextureSlot_2,
		TextureSlot_3,
		TextureSlot_4,
		TextureSlot_5,
		TextureSlot_6,
		TextureSlot_7,
		TextureSlot_8,
		TextureSlot_9,
		TextureSlot_10,
		TextureSlot_11,
		TextureSlot_12,
		TextureSlot_13,
		TextureSlot_14,
		TextureSlot_15,
		TextureSlot_16,
		TextureSlot_17,
		TextureSlot_18,
		TextureSlot_19,
		TextureSlot_20,
		TextureSlot_21,
		TextureSlot_22,
		TextureSlot_23,
		TextureSlot_24,
		TextureSlot_25,
		TextureSlot_26,
		TextureSlot_27,
		TextureSlot_28,
		TextureSlot_29,
		TextureSlot_30,
		TextureSlot_31,
	};

	class RenderCommand
	{
	private:
		RenderCommand() = delete;
		~RenderCommand() = delete;

	public:
		static constexpr int32_t TextureSlotCount = 32;
		static constexpr bool OptimizeRedundantCommands = true;

		struct State
		{
			State();

			uint32_t LastBoundShaderProgram;
			TextureSlot LastTextureSlot;
			uint32_t LastBoundTexture[TextureSlotCount];

			uint32_t& GetLastBoundTextureID();
			const uint32_t& GetLastBoundTextureID() const;
		};

	public:
		static inline const State& GetState() { return state; };
		
		static void SetClearColor(const vec4& color);
		static void Clear(ClearTarget target);

		static void SetViewport(int32_t width, int32_t height);
		static void SetViewport(ivec2 size);

		static void BindShaderProgram(uint32_t programID);

		static void SetTextureSlot(TextureSlot textureSlot);
		static void BindTexture(uint32_t textureTargetEnum, uint32_t textureID);

		static void DrawArrays(PrimitiveType primitiveType, int32_t first, int32_t count);
		static void DrawElements(PrimitiveType primitiveType, int32_t count, uint32_t typeEnum, const void* indices);

	private:
		static State state;
	};
}
