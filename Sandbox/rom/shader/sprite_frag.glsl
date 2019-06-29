#version 420 core
out vec4 fragColor;

in vec4 vertexColor;
in vec2 vertexTexCoord;

uniform bool u_solidColor;
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

vec4 GetTextureColor()
{
	if (u_textureFormat == TextureFormat_RGTC2)
	{
		vec4 grayscale = textureLod(textureSampler, vertexTexCoord, 0.0).rrrg;
		vec4 luminance = textureLod(textureSampler, vertexTexCoord, 1.0) * 1.003922 -0.503929;
		grayscale.rb = luminance.gr;
		
		return vec4(
			dot(grayscale.rgb, RED_COEF),
			dot(grayscale.rgb, GRN_COEF),
			dot(grayscale.rgb, BLU_COEF),
			grayscale.a);
	}
		
	return texture(textureSampler, vertexTexCoord);
}

vec4 GetFontTextureColor()
{
	vec2 pixelSize = 1.0 / textureSize(textureSampler, 0);
	float x = pixelSize.x; float y = pixelSize.y;

	// TEMP col, tmp; // TEMP alpha; // MOV alpha, 1.0e-9;
	vec4 col, alpha;
	
	// TEX tmp, a_tex0, texture[0], 2D;
	vec4 tmp = texture(textureSampler, vertexTexCoord);
	
	// MAX alpha.w, alpha.w, tmp.w;
	alpha.a = max(alpha.a, tmp.a);
	
	// MUL col.xyz, a_color0, tmp.w;
	col.xyz = vertexColor.rgb * tmp.aaa;
	
	// MUL col.xyz, col, a_color0.w; 
	col.xyz *= vertexColor.w;
	// TEX tmp, a_tex0, texture[0], 2D, (-1, 0);
	tmp = texture(textureSampler, vertexTexCoord + vec2(-x, 0));
	// MAX alpha.x, alpha.x, tmp.w;
	alpha.x = max(alpha.x, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, (+1, 0);
	tmp = texture(textureSampler, vertexTexCoord + vec2(+x, 0));
	//MAX alpha.x, alpha.x, tmp.w;
	alpha.x = max(alpha.x, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, ( 0,-1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(0, -y));
	// MAX alpha.x, alpha.x, tmp.w;
	alpha.x = max(alpha.x, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, ( 0,+1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(0, +y));
	// MAX alpha.x, alpha.x, tmp.w;
	alpha.x = max(alpha.x, tmp.w);

	// TEX tmp, a_tex0, texture[0], 2D, (-2, 0);
	tmp = texture(textureSampler, vertexTexCoord + vec2(-x*2, 0));
	// MAX alpha.y, alpha.y, tmp.w;
	alpha.y = max(alpha.y, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, (+2, 0);
	tmp = texture(textureSampler, vertexTexCoord + vec2(+x*2, 0));
	// MAX alpha.y, alpha.y, tmp.w;
	alpha.y = max(alpha.y, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, ( 0,-2);
	tmp = texture(textureSampler, vertexTexCoord + vec2(0, -y*2));
	// MAX alpha.y, alpha.y, tmp.w;
	alpha.y = max(alpha.y, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, ( 0,+2);
	tmp = texture(textureSampler, vertexTexCoord + vec2(0, +y*2));
	// MAX alpha.y, alpha.y, tmp.w;
	alpha.y = max(alpha.y, tmp.w);
	
	// TEX tmp, a_tex0, texture[0], 2D, (-1,-1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(-x, -y));
	// MAX alpha.z, alpha.z, tmp.w;
	alpha.z = max(alpha.z, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, (+1,-1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(+x, -y));
	// MAX alpha.z, alpha.z, tmp.w;
	alpha.z = max(alpha.z, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, (-1,+1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(-x, +y));
	// MAX alpha.z, alpha.z, tmp.w;
	alpha.z = max(alpha.z, tmp.w);
	// TEX tmp, a_tex0, texture[0], 2D, (+1,+1);
	tmp = texture(textureSampler, vertexTexCoord + vec2(+x, +y));
	// MAX alpha.z, alpha.z, tmp.w;
	alpha.z = max(alpha.z, tmp.w);
	
	// MUL alpha, alpha, { 0.9, 0.6, 0.8, 1.0 };
	alpha *= vec4(0.9, 0.6, 0.8, 1.0);

	// MAX alpha.w, alpha.w, alpha.x;
	alpha.a = max(alpha.a, alpha.r);
	// MAX alpha.w, alpha.w, alpha.y;
	alpha.a = max(alpha.a, alpha.g);
	// MAX alpha.w, alpha.w, alpha.z;
	alpha.a = max(alpha.a, alpha.b);

	// RCP tmp.w, alpha.w; ??
	tmp.w = inversesqrt(alpha.w);

	// MUL o_color.xyz, col, tmp.w;
	vec4 output = col.xyzw * tmp.wwww;
	// MUL o_color.w, alpha.w, a_color0.w;
	output.w = alpha.w * vertexColor.w;
	
	return output;
}

void main()
{
	bool u_textShadow = false; // u_textureFormat != TextureFormat_RGTC2;

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