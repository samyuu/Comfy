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
			int32_t LastTextureSlot;
			uint32_t LastBoundTexture[TextureSlotCount];

			uint32_t& GetLastBoundTextureID();
			const uint32_t& GetLastBoundTextureID() const;
		};

	public:
		static inline const State& GetState() { return state; };
		
		static void SetClearColor(const vec4& color);
		static void Clear(ClearTarget target);

		static void SetViewport(int32_t width, int32_t height);
		static void SetViewport(const vec2& size);

		static void BindShaderProgram(uint32_t programID);

		static void SetTextureSlot(int32_t textureSlot);
		static void BindTexture(uint32_t textureTargetEnum, uint32_t textureID);

		static void DrawArrays(uint32_t primitiveEnum, int32_t first, int32_t count);
		static void DrawElements(uint32_t primitiveEnum, int32_t count, uint32_t typeEnum, const void* indices);

	private:
		static State state;
	};
}
