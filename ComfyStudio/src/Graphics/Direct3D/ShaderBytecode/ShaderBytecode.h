#pragma once

#define DECLARE_BYTECODE_GETTER(shader)	BytecodeBlob shader();
#define DEFINE_BYTECODE_GETTER(shader)	BytecodeBlob shader() { return { ::shader##_Bytecode, sizeof(::shader##_Bytecode) }; }

// NOTE: Public interface
namespace Graphics
{
	struct BytecodeBlob
	{
		const void* Bytecode;
		size_t Size;
	};

	DECLARE_BYTECODE_GETTER(ImGui_VS);
	DECLARE_BYTECODE_GETTER(ImGui_PS);

	DECLARE_BYTECODE_GETTER(Sprite_VS);
	DECLARE_BYTECODE_GETTER(Sprite_PS);

	DECLARE_BYTECODE_GETTER(Test_VS);
	DECLARE_BYTECODE_GETTER(Test_PS);
}

// NOTE: Implementation included in the cpp file
#ifdef SHADER_BYTECODE_IMPLEMENTATION

#include SHADER_BYTECODE_FILE(ImGui_VS.h)
#include SHADER_BYTECODE_FILE(ImGui_PS.h)

#include SHADER_BYTECODE_FILE(Sprite_VS.h)
#include SHADER_BYTECODE_FILE(Sprite_PS.h)

#include SHADER_BYTECODE_FILE(Test_VS.h)
#include SHADER_BYTECODE_FILE(Test_PS.h)

namespace Graphics
{
	DEFINE_BYTECODE_GETTER(ImGui_VS);
	DEFINE_BYTECODE_GETTER(ImGui_PS);

	DEFINE_BYTECODE_GETTER(Sprite_VS);
	DEFINE_BYTECODE_GETTER(Sprite_PS);

	DEFINE_BYTECODE_GETTER(Test_VS);
	DEFINE_BYTECODE_GETTER(Test_PS);
}

#endif /* SHADER_BYTECODE_IMPLEMENTATION */
