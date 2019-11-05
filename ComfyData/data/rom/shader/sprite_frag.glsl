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
uniform int u_BlendMode;

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
const vec4 COLOR_MASK = vec4(1.0, 1.0, 1.0, 0.0);

// NOTE: TextureFormats
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

// NOTE: BlendModes
const int BlendMode_None = 0;
const int BlendMode_Copy = 1;
const int BlendMode_Behind = 2;
const int BlendMode_Normal = 3;
const int BlendMode_Dissolve = 4;
const int BlendMode_Add = 5;
const int BlendMode_Multiply = 6;
const int BlendMode_Screen = 7;
const int BlendMode_Overlay = 8;
const int BlendMode_SoftLight = 9;
const int BlendMode_HardLight = 10;
const int BlendMode_Darken = 11;
const int BlendMode_Lighten = 12;
const int BlendMode_ClassicDifference = 13;
const int BlendMode_Hue = 14;
const int BlendMode_Saturation = 15;
const int BlendMode_Color = 16;
const int BlendMode_Luminosity = 17;
const int BlendMode_StenciilAlpha = 18;
const int BlendMode_StencilLuma = 19;
const int BlendMode_SilhouetteAlpha = 20;
const int BlendMode_SilhouetteLuma = 21;
const int BlendMode_LuminescentPremul = 22;
const int BlendMode_AlphaAdd = 23;
const int BlendMode_ClassicColorDodge = 24;
const int BlendMode_ClassicColorBurn = 25;
const int BlendMode_Exclusion = 26;
const int BlendMode_Difference = 27;
const int BlendMode_ColorDodge = 28;
const int BlendMode_ColorBurn = 29;
const int BlendMode_LinearDodge = 30;
const int BlendMode_LinearBurn = 31;
const int BlendMode_LinearLight = 32;
const int BlendMode_VividLight = 33;
const int BlendMode_PinLight = 34;
const int BlendMode_HardMix = 35;
const int BlendMode_LighterColor = 36;
const int BlendMode_DarkerColor = 37;
const int BlendMode_Subtract = 38;
const int BlendMode_Divide = 39;

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

vec4 GetTextureColor()
{
	return GetTextureColor(u_TextureSampler, Input.TexCoord, u_TextureFormat);
}

float GetTextureAlpha(float xOffset, float yOffset)
{
	vec2 texCoord = Input.TexCoord + vec2(xOffset, yOffset);

	return (u_TextureFormat == TextureFormat_RGTC2) ?
		textureLod(u_TextureSampler, texCoord, GRAYSCALE_MIPMAP).g :
		texture(u_TextureSampler, texCoord).a;
}

vec4 GetTextureFontColor()
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

vec4 GetTextureMaskColor()
{
	// NOTE: Special case for when the texture mask shares the same texture
	if (u_TextureFormat < 0)
		return GetTextureColor(u_TextureMaskSampler, Input.TexMaskCoord, u_TextureMaskFormat);
	
	return GetTextureColor(u_TextureSampler, Input.TexMaskCoord, u_TextureFormat);
}

float GetTextureMaskAlpha()
{
	// NOTE: Because in this case we only need to sample the grayscale mipmap
	if (u_TextureMaskFormat == TextureFormat_RGTC2)
		return textureLod(u_TextureMaskSampler, Input.TexCoord, GRAYSCALE_MIPMAP).g;
	
	return texture(u_TextureMaskSampler, Input.TexCoord).a;
}

void main()
{
	if (u_SolidColor)
	{
		FragColor = Input.Color;
		
		if (u_Checkerboard)
			FragColor *= mix(1.0, 0.0, GetCheckerboardFactor());
	}
	else if (u_TextShadow)
	{
		FragColor = GetTextureFontColor();
	}
	else if (u_TextureMaskFormat >= 0)
	{
		FragColor = GetTextureMaskColor() * Input.Color;
		FragColor.a *= GetTextureMaskAlpha();
	}
	else
	{
		FragColor = GetTextureColor() * Input.Color;
	}

	if (u_BlendMode == BlendMode_Multiply)
		FragColor = ((FragColor - COLOR_MASK) * FragColor.aaaa) + COLOR_MASK;
}