#include "Types.h"

// NOTE: To automatically resolve the correct include directory based on the build configuration
#if COMFY_DEBUG
#define SHADER_BYTECODE_FILE(filename) COMFY_STRINGIFY(Intermediate/DXBC-Debug/##filename)
#elif COMFY_RELEASE
#define SHADER_BYTECODE_FILE(filename) COMFY_STRINGIFY(Intermediate/DXBC-Release/##filename)
#else
#error Unknown build configuration
#endif /* COMFY_DEBUG / COMFY_RELEASE */

// NOTE: Include BYTE typedef
#include "Render/D3D11/Direct3D.h"

// NOTE: Include implementation so only one file needs to be added every time a shader is added
#define SHADER_BYTECODE_IMPLEMENTATION
#include "ShaderBytecode.h"
