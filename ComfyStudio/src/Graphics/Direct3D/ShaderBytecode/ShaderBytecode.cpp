#define STRINGIFY(X) #X

// NOTE: To automatically resolve the correct include directory based on the build configuration
#if COMFY_DEBUG
#define SHADER_BYTECODE_FILE(filename) STRINGIFY(Debug/##filename)
#elif COMFY_RELEASE
#define SHADER_BYTECODE_FILE(filename) STRINGIFY(Release/##filename)
#else
#error Unknown build configuration
#endif /* COMFY_DEBUG / COMFY_RELEASE */

// NOTE: Include BYTE typedef
#include "Graphics/Direct3D/Direct3D.h"

// NOTE: Include implementation so only one file needs to be added every time a shader is added
#define SHADER_BYTECODE_IMPLEMENTATION
#include "ShaderBytecode.h"