#version 420 core
out vec4 fragColor;

in vec4 vertexColor;
in vec2 vertexTexCoord;

uniform bool u_solidColor;
uniform bool u_textShadow;
uniform bool u_checkerboard;
uniform vec2 u_checkboardSize;
uniform int u_textureFormat;
uniform sampler2D textureSampler;
uniform sampler2D textureMaskSampler;

const vec3 RED_COEF = { +1.5748, 1.0, +0.0000 };
const vec3 GRN_COEF = { -0.4681, 1.0, -0.1873 };
const vec3 BLU_COEF = { +0.0000, 1.0, +1.8556 };

const int TextureFormat_A8 = 0;
const int TextureFormat_RGB8 = 1;
const int TextureFormat_RGBA8 = 2;
const int TextureFormat_RGB5 = 3;
const int TextureFormat_RGB5_A1 = 4;
const int TextureFormat_RGBA4 = 5;
const int TextureFormat_DXT1 = 6;
const int TextureFormat_DXT1a = 7;
const int TextureFormat_DXT3 = 8;
const int TextureFormat_DXT5 = 9;
const int TextureFormat_RGTC1 = 10;
const int TextureFormat_RGTC2 = 11;
const int TextureFormat_L8 = 12;
const int TextureFormat_L8A8 = 13;

vec4 GetTextureColor()
{
	if (u_textureFormat == TextureFormat_RGTC2)
	{
		vec4 grayscale = textureLod(textureSampler, vertexTexCoord, 0.0).rrrg;
		vec4 luminance = textureLod(textureSampler, vertexTexCoord, 1.0) * 1.003922 - 0.503929;
		grayscale.rb = luminance.gr;
		
		return vec4(
			dot(grayscale.rgb, RED_COEF),
			dot(grayscale.rgb, GRN_COEF),
			dot(grayscale.rgb, BLU_COEF),
			grayscale.a);
	}
		
	return texture(textureSampler, vertexTexCoord);
}

float GetTextureAlpha(float xOffset, float yOffset)
{
	return (u_textureFormat == TextureFormat_RGTC2) ? 
		textureLod(textureSampler, vertexTexCoord + vec2(xOffset, yOffset), 0).g :
		texture(textureSampler, vertexTexCoord + vec2(xOffset, yOffset)).a;
}

vec4 GetFontTextureColor()
{
	vec2 size = 1.0 / textureSize(textureSampler, 0);

	float textureAlpha = GetTextureAlpha(0.0, 0.0);
	vec4 shadow = vec4(0.0, 0.0, 0.0, textureAlpha);
	
	shadow.x = max(shadow.x, GetTextureAlpha(-size.x, 0.0));
	shadow.x = max(shadow.x, GetTextureAlpha(+size.x, 0.0));
	shadow.x = max(shadow.x, GetTextureAlpha(0.0, -size.y));
	shadow.x = max(shadow.x, GetTextureAlpha(0.0, +size.y));

	shadow.y = max(shadow.y, GetTextureAlpha(-size.x * 2, 0.0));
	shadow.y = max(shadow.y, GetTextureAlpha(+size.x * 2, 0.0));
	shadow.y = max(shadow.y, GetTextureAlpha(0.0, -size.y * 2));
	shadow.y = max(shadow.y, GetTextureAlpha(0.0, +size.y * 2));
	
	shadow.z = max(shadow.z, GetTextureAlpha(-size.x, -size.y));
	shadow.z = max(shadow.z, GetTextureAlpha(+size.x, -size.y));
	shadow.z = max(shadow.z, GetTextureAlpha(-size.x, +size.y));
	shadow.z = max(shadow.z, GetTextureAlpha(+size.x, +size.y));
	
	shadow.a = max(shadow.a, shadow.x * 0.9);
	shadow.a = max(shadow.a, shadow.y * 0.6);
	shadow.a = max(shadow.a, shadow.z * 0.8);

	vec3 color = (vertexColor.rgb * textureAlpha) * vertexColor.a;
	return vec4(color * inversesqrt(shadow.a), shadow.a * vertexColor.a);
}

void main()
{
	if (u_checkerboard)
	{
		vec2 point = vertexTexCoord * u_checkboardSize;
		if ((mod(0.08 * (point.x), 1.0) < 0.5) ^^ (mod(0.08 * (point.y), 1.0) < 0.5))
			discard;
	}

	if (u_solidColor)
	{
		fragColor = vertexColor;
		return;
	}
	else if (u_textShadow)
	{ 
		fragColor = GetFontTextureColor(); 
		return; 
	}

	fragColor = GetTextureColor() * vertexColor;
}