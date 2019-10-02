#version 330 core
out vec4 FragColor;

in VS_OUT
{
	vec4 Color;
	vec2 TexCoord;
	vec2 TexMaskCoord;
} Input;

uniform bool u_SolidColor;
uniform bool u_TextShadow;
uniform bool u_Checkerboard;
uniform vec2 u_CheckboardSize;

uniform int u_TextureFormat;
uniform int u_TextureMaskFormat;

uniform sampler2D u_TextureSampler;
uniform sampler2D u_TextureMaskSampler;

const float GRAYSCALE_MIPMAP = 0.0;
const float LUMINANCE_MIPMAP = 1.0;

const vec3 RED_COEF = vec3(+1.5748, 1.0, +0.0000);
const vec3 GRN_COEF = vec3(-0.4681, 1.0, -0.1873);
const vec3 BLU_COEF = vec3(+0.0000, 1.0, +1.8556);
const float LUMINANCE_FACTOR = 1.003922;
const float LUMINANCE_OFFSET = 0.503929;

const vec3 TEXT_SHADOW_FACTOR = vec3(0.9, 0.6, 0.8);

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

vec4 GetTextureColor(sampler2D sampler, vec2 texCoord, int textureFormat)
{
	if (textureFormat == TextureFormat_RGTC2)
	{
		vec4 grayscale = textureLod(sampler, texCoord, GRAYSCALE_MIPMAP).rrrg;
		vec4 luminance = textureLod(sampler, texCoord, LUMINANCE_MIPMAP) * LUMINANCE_FACTOR - LUMINANCE_OFFSET;
		grayscale.rb = luminance.gr;
		
		return vec4(
			dot(grayscale.rgb, RED_COEF),
			dot(grayscale.rgb, GRN_COEF),
			dot(grayscale.rgb, BLU_COEF),
			grayscale.a);
	}
	
	return texture(sampler, texCoord);
}

float GetTextureAlpha(float xOffset, float yOffset)
{
	vec2 texCoord = Input.TexCoord + vec2(xOffset, yOffset);

	return (u_TextureFormat == TextureFormat_RGTC2) ?
		textureLod(u_TextureSampler, texCoord, GRAYSCALE_MIPMAP).g :
		texture(u_TextureSampler, texCoord).a;
}

vec4 GetFontTextureColor()
{
	vec2 texelSize = 1.0 / textureSize(u_TextureSampler, 0);
	vec2 texelSizeDouble = texelSize * 2.0;

	float textureAlpha = GetTextureAlpha(0.0, 0.0);
	vec4 shadow = vec4(0.0, 0.0, 0.0, textureAlpha);

	shadow.x = max(shadow.x, GetTextureAlpha(-texelSize.x, 0.0));
	shadow.x = max(shadow.x, GetTextureAlpha(+texelSize.x, 0.0));
	shadow.x = max(shadow.x, GetTextureAlpha(0.0, -texelSize.y));
	shadow.x = max(shadow.x, GetTextureAlpha(0.0, +texelSize.y));

	shadow.y = max(shadow.y, GetTextureAlpha(-texelSizeDouble.x, 0.0));
	shadow.y = max(shadow.y, GetTextureAlpha(+texelSizeDouble.x, 0.0));
	shadow.y = max(shadow.y, GetTextureAlpha(0.0, -texelSizeDouble.y));
	shadow.y = max(shadow.y, GetTextureAlpha(0.0, +texelSizeDouble.y));
	
	shadow.z = max(shadow.z, GetTextureAlpha(-texelSize.x, -texelSize.y));
	shadow.z = max(shadow.z, GetTextureAlpha(+texelSize.x, -texelSize.y));
	shadow.z = max(shadow.z, GetTextureAlpha(-texelSize.x, +texelSize.y));
	shadow.z = max(shadow.z, GetTextureAlpha(+texelSize.x, +texelSize.y));
	
	shadow.a = max(shadow.a, shadow.x * TEXT_SHADOW_FACTOR.x);
	shadow.a = max(shadow.a, shadow.y * TEXT_SHADOW_FACTOR.y);
	shadow.a = max(shadow.a, shadow.z * TEXT_SHADOW_FACTOR.z);

	vec3 color = (Input.Color.rgb * textureAlpha) * Input.Color.a;
	return vec4(color * inversesqrt(shadow.a), shadow.a * Input.Color.a);
}

float GetCheckerboardFactor()
{
	// NOTE: Offset the coordinates so we scale towards the top left corner
  	vec2 result = floor((Input.TexCoord - vec2(0.0, 1.0)) * u_CheckboardSize);
	return mod(result.x + result.y, 2.0);
}

void main()
{
	if (u_SolidColor)
	{
		FragColor = Input.Color;
	}
	else if (u_TextShadow)
	{
		FragColor = GetFontTextureColor();
	}
	else if (u_TextureMaskFormat >= 0)
	{
		// NOTE: Special case for when the texture mask shares the same texture
		if (u_TextureFormat < 0)
			FragColor = GetTextureColor(u_TextureMaskSampler, Input.TexMaskCoord, u_TextureMaskFormat);
		else
			FragColor = GetTextureColor(u_TextureSampler, Input.TexMaskCoord, u_TextureFormat);

		FragColor *= Input.Color;
		FragColor.a *= GetTextureColor(u_TextureMaskSampler, Input.TexCoord, u_TextureMaskFormat).a;
	}
	else
	{
		FragColor = GetTextureColor(u_TextureSampler, Input.TexCoord, u_TextureFormat) * Input.Color;
	}

	if (u_Checkerboard)
	{
		FragColor *= mix(1.0, 0.0, GetCheckerboardFactor());
	}
}