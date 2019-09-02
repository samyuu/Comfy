#version 420 core
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

vec4 GetTextureColor(sampler2D sampler, vec2 texCoord, int textureFormat)
{
	if (textureFormat == TextureFormat_RGTC2)
	{
		vec4 grayscale = textureLod(sampler, texCoord, 0.0).rrrg;
		vec4 luminance = textureLod(sampler, texCoord, 1.0) * 1.003922 - 0.503929;
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
	return (u_TextureFormat == TextureFormat_RGTC2) ? 
		textureLod(u_TextureSampler, Input.TexCoord + vec2(xOffset, yOffset), 0).g :
		texture(u_TextureSampler, Input.TexCoord + vec2(xOffset, yOffset)).a;
}

vec4 GetFontTextureColor()
{
	vec2 size = 1.0 / textureSize(u_TextureSampler, 0);

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

	vec3 color = (Input.Color.rgb * textureAlpha) * Input.Color.a;
	return vec4(color * inversesqrt(shadow.a), shadow.a * Input.Color.a);
}

void main()
{
	if (u_SolidColor)
	{
		if (u_Checkerboard)
		{
			vec2 point = (Input.TexCoord - vec2(0.0, 1.0)) * u_CheckboardSize * 0.1;
			if ((mod(point.x, 1.0) < 0.5) ^^ (mod(point.y, 1.0) < 0.5))
				discard;
		}

		FragColor = Input.Color;
	}
	else if (u_TextShadow)
	{ 
		FragColor = GetFontTextureColor();
	}
	else if (u_TextureMaskFormat >= 0)
	{
		FragColor = GetTextureColor(u_TextureSampler, Input.TexMaskCoord, u_TextureFormat) * Input.Color;
		FragColor.a *= GetTextureColor(u_TextureMaskSampler, Input.TexCoord, u_TextureMaskFormat).a;
	}
	else
	{
		FragColor = GetTextureColor(u_TextureSampler, Input.TexCoord, u_TextureFormat) * Input.Color;
	}
}