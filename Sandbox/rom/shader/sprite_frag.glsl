#version 420 core
out vec4 fragColor;

in vec4 vertexColor;
in vec2 vertexTexCoord;

uniform int u_textureFormat;
uniform sampler2D textureSampler;
uniform sampler2D textureMaskSampler;

const vec3 RED_COEF = { +1.5748, 1.0, +0.0000 };
const vec3 GRN_COEF = { -0.4681, 1.0, -0.1873 };
const vec3 BLU_COEF = { +0.0000, 1.0, +1.8556 };

// GL_ALPHA8
const int TextureFormat_A8 = 0;
// GL_RGB8
const int TextureFormat_RGB8 = 1;
// GL_RGBA8
const int TextureFormat_RGBA8 = 2;
// GL_RGB5
const int TextureFormat_RGB5 = 3;
// GL_RGB5_A1
const int TextureFormat_RGB5_A1 = 4;
// GL_RGBA4
const int TextureFormat_RGBA4 = 5;
// GL_COMPRESSED_RGB_S3TC_DXT1_EXT
const int TextureFormat_DXT1 = 6;
// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
const int TextureFormat_DXT1a = 7;
// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
const int TextureFormat_DXT3 = 8;
// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
const int TextureFormat_DXT5 = 9;
// GL_COMPRESSED_RED_RGTC1
const int TextureFormat_RGTC1 = 10;
// GL_COMPRESSED_RG_RGTC2
const int TextureFormat_RGTC2 = 11;
// GL_LUMINANCE8
const int TextureFormat_L8 = 12;
// GL_LUMINANCE8_ALPHA8
const int TextureFormat_L8A8 = 13;
	
void main()
{
	vec4 textureColor;

	if (u_textureFormat == TextureFormat_RGTC2)
	{
		vec4 grayscale = textureLod(textureSampler, vertexTexCoord, 0.0).rrrg;
		vec4 luminance = textureLod(textureSampler, vertexTexCoord, 1.0) * 1.003922 -0.503929;
		grayscale.rb = luminance.gr;
		textureColor.r = dot(grayscale.rgb, RED_COEF);
		textureColor.g = dot(grayscale.rgb, GRN_COEF);
		textureColor.b = dot(grayscale.rgb, BLU_COEF);
		textureColor.a = grayscale.a;
	}
	else
	{
		textureColor = texture(textureSampler, vertexTexCoord);
	}

	fragColor = textureColor * vertexColor;
}