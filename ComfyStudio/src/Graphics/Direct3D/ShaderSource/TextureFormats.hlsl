#ifndef TEXTUREFORMATS_HLSL
#define TEXTUREFORMATS_HLSL

typedef int TextureFormat;

// NOTE: TextureFormats
static const TextureFormat TextureFormat_Unknown = -1;
static const TextureFormat TextureFormat_A8 = 0;
static const TextureFormat TextureFormat_RGB8 = 1;
static const TextureFormat TextureFormat_RGBA8 = 2;
static const TextureFormat TextureFormat_RGB5 = 3;
static const TextureFormat TextureFormat_RGB5_A1 = 4;
static const TextureFormat TextureFormat_RGBA4 = 5;
static const TextureFormat TextureFormat_DXT1 = 6;
static const TextureFormat TextureFormat_DXT1a = 7;
static const TextureFormat TextureFormat_DXT3 = 8;
static const TextureFormat TextureFormat_DXT5 = 9;
static const TextureFormat TextureFormat_RGTC1 = 10;
static const TextureFormat TextureFormat_RGTC2 = 11;
static const TextureFormat TextureFormat_L8 = 12;
static const TextureFormat TextureFormat_L8A8 = 13;

#endif /* TEXTUREFORMATS_HLSL */
