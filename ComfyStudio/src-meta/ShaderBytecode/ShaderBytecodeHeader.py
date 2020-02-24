from io import open

inputFileName = "ShaderList.txt"
outputFileName = "../../src/Graphics/Direct3D/ShaderBytecode/ShaderBytecode.h"

shaderNamesToInclude = []
with open(inputFileName, 'r') as inputFile:
	shaderNamesToInclude = inputFile.read().splitlines()

outputFileContent = "#pragma once\n"
outputFileContent += "#include \"Graphics/Direct3D/D3D_Shader.h\"\n"
outputFileContent += "\n"
outputFileContent += "#define DECLARE_BYTECODE_GETTER(shader)	BytecodeBlob shader();\n"
outputFileContent += "#define DEFINE_BYTECODE_GETTER(shader)	BytecodeBlob shader() { return { ::shader##_Bytecode, sizeof(::shader##_Bytecode) }; }\n"
outputFileContent += "\n"
outputFileContent += "// NOTE: Public interface\n"
outputFileContent += "namespace Graphics\n"
outputFileContent += "{\n"
for name in shaderNamesToInclude:
	outputFileContent += f"\tDECLARE_BYTECODE_GETTER({name});\n"
outputFileContent += "}\n"
outputFileContent += "\n"
outputFileContent += "// NOTE: Implementation included in the cpp file\n"
outputFileContent += "#ifdef SHADER_BYTECODE_IMPLEMENTATION\n"
outputFileContent += "\n"
for name in shaderNamesToInclude:
	outputFileContent += f"#include SHADER_BYTECODE_FILE({name}.h)\n"
outputFileContent += "\n"
outputFileContent += "namespace Graphics\n"
outputFileContent += "{\n"
for name in shaderNamesToInclude:
	outputFileContent += f"\tDEFINE_BYTECODE_GETTER({name});\n"
outputFileContent += "}\n"
outputFileContent += "\n"
outputFileContent += "#endif /* SHADER_BYTECODE_IMPLEMENTATION */\n"

with open(outputFileName, 'w') as outputFile:
	outputFile.write(outputFileContent)
