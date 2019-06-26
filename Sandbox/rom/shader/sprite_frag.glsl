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

const int TextureFormat_RGB = 1;
const int TextureFormat_RGBA = 2;
const int TextureFormat_RGBA4 = 5;
const int TextureFormat_DXT1 = 6;
const int TextureFormat_DXT3 = 7;
const int TextureFormat_DXT4 = 8;
const int TextureFormat_DXT5 = 9;
const int TextureFormat_ATI1 = 10;
const int TextureFormat_ATI2 = 11;

void main()
{
	vec4 textureColor;

	if (u_textureFormat == TextureFormat_ATI2)
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