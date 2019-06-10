#include "decompress.h"
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

// https://github.com/baldurk/visor/blob/master/3rdparty/decompress.c

/*
DXT1/DXT3/DXT5 texture decompression

The original code is from Benjamin Dobell, see below for details. Compared to
the original the code is now valid C89, has support for 64-bit architectures
and has been refactored. It also has support for additional formats and uses
a different PackRGBA order.

---

Copyright (c) 2012 - 2015 Matthäus G. "Anteru" Chajdas (http://anteru.net)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

Copyright (C) 2009 Benjamin Dobell, Glass Echidna

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---
*/
static uint32_t PackRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return r | (g << 8) | (b << 16) | (a << 24);
}

static float Int8ToFloat_SNORM(const uint8_t input)
{
	return (float)((int8_t)input) / 127.0f;
}

static float Int8ToFloat_UNORM(const uint8_t input)
{
	return (float)input / 255.0f;
}

/**
Decompress a BC 16x3 index block stored as
h g f e
d c b a
p o n m
l k j i

Bits packed as

| h | g | f | e | d | c | b | a | // Entry
|765 432 107 654 321 076 543 210| // Bit
|0000000000111111111112222222222| // Byte

into 16 8-bit indices.
*/
static void Decompress16x3bitIndices(const uint8_t* packed, uint8_t* unpacked)
{
	uint32_t tmp, block, i;

	for (block = 0; block < 2; ++block)
	{
		tmp = 0;

		// Read three bytes
		for (i = 0; i < 3; ++i)
		{
			tmp |= ((uint32_t)packed[i]) << (i * 8);
		}

		// Unpack 8x3 bit from last 3 byte block
		for (i = 0; i < 8; ++i)
		{
			unpacked[i] = (tmp >> (i * 3)) & 0x7;
		}

		packed += 3;
		unpacked += 8;
	}
}

static void DecompressBlockBC1Internal(uint32_t x, uint32_t y, const uint8_t* block, unsigned char* output, uint32_t outputStride, const uint8_t* alphaValues)
{
	uint32_t temp, code;

	uint16_t color0, color1;
	uint8_t r0, g0, b0, r1, g1, b1;

	int i, j;

	color0 = *(const uint16_t*)(block);
	color1 = *(const uint16_t*)(block + 2);

	temp = (color0 >> 11) * 255 + 16;
	r0 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	g0 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	b0 = (uint8_t)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	r1 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	g1 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	b1 = (uint8_t)((temp / 32 + temp) / 32);

	code = *(const uint32_t*)(block + 4);

	if (color0 > color1)
	{
		for (j = 0; j < 4; ++j)
		{
			for (i = 0; i < 4; ++i)
			{
				uint32_t finalColor, positionCode;
				uint8_t alpha;

				alpha = alphaValues[j * 4 + i];

				finalColor = 0;
				positionCode = (code >> 2 * (4 * j + i)) & 0x03;

				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, alpha);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, alpha);
					break;
				case 2:
					finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, alpha);
					break;
				case 3:
					finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, alpha);
					break;
				}

				//if (x + i < outputStride)
				((unsigned long*)output)[(y + j)*outputStride + (x + i)] = finalColor;
			}
		}
	}
	else
	{
		for (j = 0; j < 4; ++j)
		{
			for (i = 0; i < 4; ++i)
			{
				uint32_t finalColor, positionCode;
				uint8_t alpha;

				alpha = alphaValues[j * 4 + i];

				finalColor = 0;
				positionCode = (code >> 2 * (4 * j + i)) & 0x03;

				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, alpha);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, alpha);
					break;
				case 2:
					finalColor = PackRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, alpha);
					break;
				case 3:
					finalColor = PackRGBA(0, 0, 0, alpha);
					break;
				}

				//if (x + i < outputStride)
				((unsigned long*)output)[(y + j)*outputStride + (x + i)] = finalColor;
			}
		}
	}
}

/*
Decompresses one block of a BC1 (DXT1) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
uint32_t* image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC1(uint32_t x, uint32_t y, uint32_t stride,
	const uint8_t* blockStorage, unsigned char* image)
{
	static const uint8_t const_alpha[] = {
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255
	};

	DecompressBlockBC1Internal(x, y, blockStorage, image + x * sizeof(uint32_t) + (y * stride), stride, const_alpha);
}

/*
Decompresses one block of a BC3 (DXT5) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC3(uint32_t x, uint32_t y, uint32_t stride,
	const uint8_t* blockStorage, unsigned char* image)
{
	uint8_t alpha0, alpha1;
	uint8_t alphaIndices[16];

	uint16_t color0, color1;
	uint8_t r0, g0, b0, r1, g1, b1;

	int i, j;

	uint32_t temp, code;

	alpha0 = *(blockStorage);
	alpha1 = *(blockStorage + 1);

	Decompress16x3bitIndices(blockStorage + 2, alphaIndices);

	color0 = *(const uint16_t*)(blockStorage + 8);
	color1 = *(const uint16_t*)(blockStorage + 10);

	temp = (color0 >> 11) * 255 + 16;
	r0 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	g0 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	b0 = (uint8_t)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	r1 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	g1 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	b1 = (uint8_t)((temp / 32 + temp) / 32);

	code = *(const uint32_t*)(blockStorage + 12);

	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 4; i++)
		{
			uint8_t finalAlpha;
			int alphaCode;
			uint8_t colorCode;
			uint32_t finalColor;

			alphaCode = alphaIndices[4 * j + i];

			if (alphaCode == 0)
			{
				finalAlpha = alpha0;
			}
			else if (alphaCode == 1)
			{
				finalAlpha = alpha1;
			}
			else
			{
				if (alpha0 > alpha1)
				{
					finalAlpha = (uint8_t)(((8 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 7);
				}
				else
				{
					if (alphaCode == 6)
					{
						finalAlpha = 0;
					}
					else if (alphaCode == 7)
					{
						finalAlpha = 255;
					}
					else
					{
						finalAlpha = (uint8_t)(((6 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 5);
					}
				}
			}

			colorCode = (code >> 2 * (4 * j + i)) & 0x03;
			finalColor = 0;

			switch (colorCode)
			{
			case 0:
				finalColor = PackRGBA(r0, g0, b0, finalAlpha);
				break;
			case 1:
				finalColor = PackRGBA(r1, g1, b1, finalAlpha);
				break;
			case 2:
				finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, finalAlpha);
				break;
			case 3:
				finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, finalAlpha);
				break;
			}


			//*(uint32_t*)(image + sizeof(uint32_t) * (i + x) + (stride * (y + j))) = finalColor;
			((uint32_t*)image)[(y + j)*stride + (x + i)] = finalColor;
			//((unsigned long*)image)[(y + j)*stride + (x + i)] = finalColor;

		}
	}
}

/*
Decompresses one block of a BC2 (DXT3) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC2(uint32_t x, uint32_t y, uint32_t stride,
	const uint8_t* blockStorage, unsigned char* image)
{
	int i;

	uint8_t alphaValues[16] = { 0 };

	for (i = 0; i < 4; ++i)
	{
		const uint16_t* alphaData = (const uint16_t*)(blockStorage);

		alphaValues[i * 4 + 0] = (((*alphaData) >> 0) & 0xF) * 17;
		alphaValues[i * 4 + 1] = (((*alphaData) >> 4) & 0xF) * 17;
		alphaValues[i * 4 + 2] = (((*alphaData) >> 8) & 0xF) * 17;
		alphaValues[i * 4 + 3] = (((*alphaData) >> 12) & 0xF) * 17;

		blockStorage += 2;
	}

	DecompressBlockBC1Internal(x, y, blockStorage, image + x * sizeof(uint32_t) + (y * stride), stride, alphaValues);
}

enum BC4Mode
{
	BC4_UNORM = 0,
	BC4_SNORM = 1
};

enum BC5Mode
{
	BC5_UNORM = 0,
	BC5_SNORM = 1
};

static void DecompressBlockBC4Internal(
	const uint8_t* block, unsigned char* output,
	uint32_t outputStride, const float* colorTable)
{
	uint8_t indices[16];
	int x, y;

	Decompress16x3bitIndices(block + 2, indices);

	for (y = 0; y < 4; ++y)
	{
		for (x = 0; x < 4; ++x)
		{
			*(float*)(output + x * sizeof(float)) = colorTable[indices[y * 4 + x]];
		}

		output += outputStride;
	}
}

/*
Decompresses one block of a BC4 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
float* image:					pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC4(uint32_t x, uint32_t y, uint32_t stride, enum BC4Mode mode,
	const uint8_t* blockStorage, unsigned char* image)
{
	float colorTable[8];
	float r0, r1;

	if (mode == BC4_UNORM)
	{
		r0 = Int8ToFloat_UNORM(blockStorage[0]);
		r1 = Int8ToFloat_UNORM(blockStorage[1]);

		colorTable[0] = r0;
		colorTable[1] = r1;

		if (r0 > r1)
		{
			// 6 interpolated color values
			colorTable[2] = (6 * r0 + 1 * r1) / 7.0f; // bit code 010
			colorTable[3] = (5 * r0 + 2 * r1) / 7.0f; // bit code 011
			colorTable[4] = (4 * r0 + 3 * r1) / 7.0f; // bit code 100
			colorTable[5] = (3 * r0 + 4 * r1) / 7.0f; // bit code 101
			colorTable[6] = (2 * r0 + 5 * r1) / 7.0f; // bit code 110
			colorTable[7] = (1 * r0 + 6 * r1) / 7.0f; // bit code 111
		}
		else
		{
			// 4 interpolated color values
			colorTable[2] = (4 * r0 + 1 * r1) / 5.0f; // bit code 010
			colorTable[3] = (3 * r0 + 2 * r1) / 5.0f; // bit code 011
			colorTable[4] = (2 * r0 + 3 * r1) / 5.0f; // bit code 100
			colorTable[5] = (1 * r0 + 4 * r1) / 5.0f; // bit code 101
			colorTable[6] = 0.0f;               // bit code 110
			colorTable[7] = 1.0f;               // bit code 111
		}
	}
	else if (mode == BC4_SNORM)
	{
		r0 = Int8ToFloat_SNORM(blockStorage[0]);
		r1 = Int8ToFloat_SNORM(blockStorage[1]);

		colorTable[0] = r0;
		colorTable[1] = r1;

		if (r0 > r1)
		{
			// 6 interpolated color values
			colorTable[2] = (6 * r0 + 1 * r1) / 7.0f; // bit code 010
			colorTable[3] = (5 * r0 + 2 * r1) / 7.0f; // bit code 011
			colorTable[4] = (4 * r0 + 3 * r1) / 7.0f; // bit code 100
			colorTable[5] = (3 * r0 + 4 * r1) / 7.0f; // bit code 101
			colorTable[6] = (2 * r0 + 5 * r1) / 7.0f; // bit code 110
			colorTable[7] = (1 * r0 + 6 * r1) / 7.0f; // bit code 111
		}
		else
		{
			// 4 interpolated color values
			colorTable[2] = (4 * r0 + 1 * r1) / 5.0f; // bit code 010
			colorTable[3] = (3 * r0 + 2 * r1) / 5.0f; // bit code 011
			colorTable[4] = (2 * r0 + 3 * r1) / 5.0f; // bit code 100
			colorTable[5] = (1 * r0 + 4 * r1) / 5.0f; // bit code 101
			colorTable[6] = -1.0f;              // bit code 110
			colorTable[7] = 1.0f;              // bit code 111
		}
	}

	DecompressBlockBC4Internal(blockStorage,
		image + x * sizeof(float) + (y * stride), stride, colorTable);
}

uint32_t *crntImgPtr, *crntImgPtrEnd;
/*
Decompresses one block of a BC5 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
float* image:					pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC5(uint32_t x, uint32_t y, uint32_t stride, enum BC5Mode mode, const uint8_t* blockStorage, unsigned char* image)
{
	// We decompress the two channels separately and interleave them when
	// writing to the output
	// float c0[16];
	// float c1[16];

	uint32_t c0[16];
	uint32_t c1[16];

	int dx, dy;

	DecompressBlockBC4(0, 0, 4 * sizeof(float), (enum BC4Mode)mode,
		blockStorage, (unsigned char*)c0);
	DecompressBlockBC4(0, 0, 4 * sizeof(float), (enum BC4Mode)mode,
		blockStorage + 4, (unsigned char*)c1);

	int i = 0;
	for (dy = 0; dy < 4; ++dy)
	{
		for (dx = 0; dx < 4; ++dx)
		{
			//*(float*)(image + stride * y + (x + dx + 0) * sizeof(float) * 2) = c0[dy * 4 + dx];
			//*(float*)(image + stride * y + (x + dx + 1) * sizeof(float) * 2) = c1[dy * 4 + dx];

			int index = dy * 4 + dx;
			//int index = dx * 4 + dy;

			uint32_t finalColor0 = c0[index];
			uint32_t finalColor1 = c1[index];

			//if (crntImgPtr < crntImgPtrEnd)
			//	*(crntImgPtr++) = finalColor0;
			//if (crntImgPtr < crntImgPtrEnd)
			//	*(crntImgPtr++) = finalColor1;
			//else
			//{
			//	printf("x: %d y: %d\n", x, y);
			//}
			float flt0 = *(float*)&finalColor0;
			float flt1 = *(float*)&finalColor1;
			
			uint8_t grayScale = flt0 * 0xFF;
			uint8_t alpha = flt1 * 0xFF;

			//if (crntImgPtr < crntImgPtrEnd)
				*(crntImgPtr++) = PackRGBA(grayScale, grayScale, grayScale, alpha);

			// printf("%f %f\n", flt0, flt1);
			continue;

			// uint32_t testColor = PackRGBA(0xFF, 0x00, 0x00, 0xFF);
			// finalColor0 =&testColor;

			//*(float*)(image + stride * y + (x + dx + 0) * sizeof(float)) = finalColor0;
			//*(float*)(image + stride * y + (x + dx + 1) * sizeof(float)) = finalColor1;

			uint32_t* imageOffset0 = (uint32_t*)(image + stride * y + (x + dx + 0) * sizeof(uint32_t));
			uint32_t* imageOffset1 = (uint32_t*)(image + stride * y + (x + dx + 1) * sizeof(uint32_t));

			*imageOffset0 = finalColor0;
			*imageOffset1 = finalColor1;

			int64_t relativeOffset0 = (int64_t)imageOffset0 - (int64_t)image;
			int64_t relativeOffset1 = (int64_t)imageOffset1 - (int64_t)image;

			//printf("relative: %d - %d\n", relativeOffset0, relativeOffset1);

			int __breakpoint;
			//*(uint32_t*)(image + sizeof(uint32_t) * (i + x) + (stride * (y + j))) = finalColor;
			//((uint32_t*)image)[(y + dx)*stride + (x + dy + 0)] = c0[dy * 4 + dx];
			//((uint32_t*)image)[(y + dx)*stride + (x + dy + 1)] = c0[dy * 4 + dx];
		}
	}
}

// ---------------------------------------------------------------------------------------------
// https://github.com/hglm/detex/blob/master/decompress-rgtc.c
// ---------------------------------------------------------------------------------------------
const uint8_t detex_division_by_7_table[1792] = {
	0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 2, 2,
	2, 2, 2, 2, 2, 3, 3, 3,
	3, 3, 3, 3, 4, 4, 4, 4,
	4, 4, 4, 5, 5, 5, 5, 5,
	5, 5, 6, 6, 6, 6, 6, 6,
	6, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 9,
	9, 9, 9, 9, 9, 9, 10, 10,
	10, 10, 10, 10, 10, 11, 11, 11,
	11, 11, 11, 11, 12, 12, 12, 12,
	12, 12, 12, 13, 13, 13, 13, 13,
	13, 13, 14, 14, 14, 14, 14, 14,
	14, 15, 15, 15, 15, 15, 15, 15,
	16, 16, 16, 16, 16, 16, 16, 17,
	17, 17, 17, 17, 17, 17, 18, 18,
	18, 18, 18, 18, 18, 19, 19, 19,
	19, 19, 19, 19, 20, 20, 20, 20,
	20, 20, 20, 21, 21, 21, 21, 21,
	21, 21, 22, 22, 22, 22, 22, 22,
	22, 23, 23, 23, 23, 23, 23, 23,
	24, 24, 24, 24, 24, 24, 24, 25,
	25, 25, 25, 25, 25, 25, 26, 26,
	26, 26, 26, 26, 26, 27, 27, 27,
	27, 27, 27, 27, 28, 28, 28, 28,
	28, 28, 28, 29, 29, 29, 29, 29,
	29, 29, 30, 30, 30, 30, 30, 30,
	30, 31, 31, 31, 31, 31, 31, 31,
	32, 32, 32, 32, 32, 32, 32, 33,
	33, 33, 33, 33, 33, 33, 34, 34,
	34, 34, 34, 34, 34, 35, 35, 35,
	35, 35, 35, 35, 36, 36, 36, 36,
	36, 36, 36, 37, 37, 37, 37, 37,
	37, 37, 38, 38, 38, 38, 38, 38,
	38, 39, 39, 39, 39, 39, 39, 39,
	40, 40, 40, 40, 40, 40, 40, 41,
	41, 41, 41, 41, 41, 41, 42, 42,
	42, 42, 42, 42, 42, 43, 43, 43,
	43, 43, 43, 43, 44, 44, 44, 44,
	44, 44, 44, 45, 45, 45, 45, 45,
	45, 45, 46, 46, 46, 46, 46, 46,
	46, 47, 47, 47, 47, 47, 47, 47,
	48, 48, 48, 48, 48, 48, 48, 49,
	49, 49, 49, 49, 49, 49, 50, 50,
	50, 50, 50, 50, 50, 51, 51, 51,
	51, 51, 51, 51, 52, 52, 52, 52,
	52, 52, 52, 53, 53, 53, 53, 53,
	53, 53, 54, 54, 54, 54, 54, 54,
	54, 55, 55, 55, 55, 55, 55, 55,
	56, 56, 56, 56, 56, 56, 56, 57,
	57, 57, 57, 57, 57, 57, 58, 58,
	58, 58, 58, 58, 58, 59, 59, 59,
	59, 59, 59, 59, 60, 60, 60, 60,
	60, 60, 60, 61, 61, 61, 61, 61,
	61, 61, 62, 62, 62, 62, 62, 62,
	62, 63, 63, 63, 63, 63, 63, 63,
	64, 64, 64, 64, 64, 64, 64, 65,
	65, 65, 65, 65, 65, 65, 66, 66,
	66, 66, 66, 66, 66, 67, 67, 67,
	67, 67, 67, 67, 68, 68, 68, 68,
	68, 68, 68, 69, 69, 69, 69, 69,
	69, 69, 70, 70, 70, 70, 70, 70,
	70, 71, 71, 71, 71, 71, 71, 71,
	72, 72, 72, 72, 72, 72, 72, 73,
	73, 73, 73, 73, 73, 73, 74, 74,
	74, 74, 74, 74, 74, 75, 75, 75,
	75, 75, 75, 75, 76, 76, 76, 76,
	76, 76, 76, 77, 77, 77, 77, 77,
	77, 77, 78, 78, 78, 78, 78, 78,
	78, 79, 79, 79, 79, 79, 79, 79,
	80, 80, 80, 80, 80, 80, 80, 81,
	81, 81, 81, 81, 81, 81, 82, 82,
	82, 82, 82, 82, 82, 83, 83, 83,
	83, 83, 83, 83, 84, 84, 84, 84,
	84, 84, 84, 85, 85, 85, 85, 85,
	85, 85, 86, 86, 86, 86, 86, 86,
	86, 87, 87, 87, 87, 87, 87, 87,
	88, 88, 88, 88, 88, 88, 88, 89,
	89, 89, 89, 89, 89, 89, 90, 90,
	90, 90, 90, 90, 90, 91, 91, 91,
	91, 91, 91, 91, 92, 92, 92, 92,
	92, 92, 92, 93, 93, 93, 93, 93,
	93, 93, 94, 94, 94, 94, 94, 94,
	94, 95, 95, 95, 95, 95, 95, 95,
	96, 96, 96, 96, 96, 96, 96, 97,
	97, 97, 97, 97, 97, 97, 98, 98,
	98, 98, 98, 98, 98, 99, 99, 99,
	99, 99, 99, 99, 100, 100, 100, 100,
	100, 100, 100, 101, 101, 101, 101, 101,
	101, 101, 102, 102, 102, 102, 102, 102,
	102, 103, 103, 103, 103, 103, 103, 103,
	104, 104, 104, 104, 104, 104, 104, 105,
	105, 105, 105, 105, 105, 105, 106, 106,
	106, 106, 106, 106, 106, 107, 107, 107,
	107, 107, 107, 107, 108, 108, 108, 108,
	108, 108, 108, 109, 109, 109, 109, 109,
	109, 109, 110, 110, 110, 110, 110, 110,
	110, 111, 111, 111, 111, 111, 111, 111,
	112, 112, 112, 112, 112, 112, 112, 113,
	113, 113, 113, 113, 113, 113, 114, 114,
	114, 114, 114, 114, 114, 115, 115, 115,
	115, 115, 115, 115, 116, 116, 116, 116,
	116, 116, 116, 117, 117, 117, 117, 117,
	117, 117, 118, 118, 118, 118, 118, 118,
	118, 119, 119, 119, 119, 119, 119, 119,
	120, 120, 120, 120, 120, 120, 120, 121,
	121, 121, 121, 121, 121, 121, 122, 122,
	122, 122, 122, 122, 122, 123, 123, 123,
	123, 123, 123, 123, 124, 124, 124, 124,
	124, 124, 124, 125, 125, 125, 125, 125,
	125, 125, 126, 126, 126, 126, 126, 126,
	126, 127, 127, 127, 127, 127, 127, 127,
	128, 128, 128, 128, 128, 128, 128, 129,
	129, 129, 129, 129, 129, 129, 130, 130,
	130, 130, 130, 130, 130, 131, 131, 131,
	131, 131, 131, 131, 132, 132, 132, 132,
	132, 132, 132, 133, 133, 133, 133, 133,
	133, 133, 134, 134, 134, 134, 134, 134,
	134, 135, 135, 135, 135, 135, 135, 135,
	136, 136, 136, 136, 136, 136, 136, 137,
	137, 137, 137, 137, 137, 137, 138, 138,
	138, 138, 138, 138, 138, 139, 139, 139,
	139, 139, 139, 139, 140, 140, 140, 140,
	140, 140, 140, 141, 141, 141, 141, 141,
	141, 141, 142, 142, 142, 142, 142, 142,
	142, 143, 143, 143, 143, 143, 143, 143,
	144, 144, 144, 144, 144, 144, 144, 145,
	145, 145, 145, 145, 145, 145, 146, 146,
	146, 146, 146, 146, 146, 147, 147, 147,
	147, 147, 147, 147, 148, 148, 148, 148,
	148, 148, 148, 149, 149, 149, 149, 149,
	149, 149, 150, 150, 150, 150, 150, 150,
	150, 151, 151, 151, 151, 151, 151, 151,
	152, 152, 152, 152, 152, 152, 152, 153,
	153, 153, 153, 153, 153, 153, 154, 154,
	154, 154, 154, 154, 154, 155, 155, 155,
	155, 155, 155, 155, 156, 156, 156, 156,
	156, 156, 156, 157, 157, 157, 157, 157,
	157, 157, 158, 158, 158, 158, 158, 158,
	158, 159, 159, 159, 159, 159, 159, 159,
	160, 160, 160, 160, 160, 160, 160, 161,
	161, 161, 161, 161, 161, 161, 162, 162,
	162, 162, 162, 162, 162, 163, 163, 163,
	163, 163, 163, 163, 164, 164, 164, 164,
	164, 164, 164, 165, 165, 165, 165, 165,
	165, 165, 166, 166, 166, 166, 166, 166,
	166, 167, 167, 167, 167, 167, 167, 167,
	168, 168, 168, 168, 168, 168, 168, 169,
	169, 169, 169, 169, 169, 169, 170, 170,
	170, 170, 170, 170, 170, 171, 171, 171,
	171, 171, 171, 171, 172, 172, 172, 172,
	172, 172, 172, 173, 173, 173, 173, 173,
	173, 173, 174, 174, 174, 174, 174, 174,
	174, 175, 175, 175, 175, 175, 175, 175,
	176, 176, 176, 176, 176, 176, 176, 177,
	177, 177, 177, 177, 177, 177, 178, 178,
	178, 178, 178, 178, 178, 179, 179, 179,
	179, 179, 179, 179, 180, 180, 180, 180,
	180, 180, 180, 181, 181, 181, 181, 181,
	181, 181, 182, 182, 182, 182, 182, 182,
	182, 183, 183, 183, 183, 183, 183, 183,
	184, 184, 184, 184, 184, 184, 184, 185,
	185, 185, 185, 185, 185, 185, 186, 186,
	186, 186, 186, 186, 186, 187, 187, 187,
	187, 187, 187, 187, 188, 188, 188, 188,
	188, 188, 188, 189, 189, 189, 189, 189,
	189, 189, 190, 190, 190, 190, 190, 190,
	190, 191, 191, 191, 191, 191, 191, 191,
	192, 192, 192, 192, 192, 192, 192, 193,
	193, 193, 193, 193, 193, 193, 194, 194,
	194, 194, 194, 194, 194, 195, 195, 195,
	195, 195, 195, 195, 196, 196, 196, 196,
	196, 196, 196, 197, 197, 197, 197, 197,
	197, 197, 198, 198, 198, 198, 198, 198,
	198, 199, 199, 199, 199, 199, 199, 199,
	200, 200, 200, 200, 200, 200, 200, 201,
	201, 201, 201, 201, 201, 201, 202, 202,
	202, 202, 202, 202, 202, 203, 203, 203,
	203, 203, 203, 203, 204, 204, 204, 204,
	204, 204, 204, 205, 205, 205, 205, 205,
	205, 205, 206, 206, 206, 206, 206, 206,
	206, 207, 207, 207, 207, 207, 207, 207,
	208, 208, 208, 208, 208, 208, 208, 209,
	209, 209, 209, 209, 209, 209, 210, 210,
	210, 210, 210, 210, 210, 211, 211, 211,
	211, 211, 211, 211, 212, 212, 212, 212,
	212, 212, 212, 213, 213, 213, 213, 213,
	213, 213, 214, 214, 214, 214, 214, 214,
	214, 215, 215, 215, 215, 215, 215, 215,
	216, 216, 216, 216, 216, 216, 216, 217,
	217, 217, 217, 217, 217, 217, 218, 218,
	218, 218, 218, 218, 218, 219, 219, 219,
	219, 219, 219, 219, 220, 220, 220, 220,
	220, 220, 220, 221, 221, 221, 221, 221,
	221, 221, 222, 222, 222, 222, 222, 222,
	222, 223, 223, 223, 223, 223, 223, 223,
	224, 224, 224, 224, 224, 224, 224, 225,
	225, 225, 225, 225, 225, 225, 226, 226,
	226, 226, 226, 226, 226, 227, 227, 227,
	227, 227, 227, 227, 228, 228, 228, 228,
	228, 228, 228, 229, 229, 229, 229, 229,
	229, 229, 230, 230, 230, 230, 230, 230,
	230, 231, 231, 231, 231, 231, 231, 231,
	232, 232, 232, 232, 232, 232, 232, 233,
	233, 233, 233, 233, 233, 233, 234, 234,
	234, 234, 234, 234, 234, 235, 235, 235,
	235, 235, 235, 235, 236, 236, 236, 236,
	236, 236, 236, 237, 237, 237, 237, 237,
	237, 237, 238, 238, 238, 238, 238, 238,
	238, 239, 239, 239, 239, 239, 239, 239,
	240, 240, 240, 240, 240, 240, 240, 241,
	241, 241, 241, 241, 241, 241, 242, 242,
	242, 242, 242, 242, 242, 243, 243, 243,
	243, 243, 243, 243, 244, 244, 244, 244,
	244, 244, 244, 245, 245, 245, 245, 245,
	245, 245, 246, 246, 246, 246, 246, 246,
	246, 247, 247, 247, 247, 247, 247, 247,
	248, 248, 248, 248, 248, 248, 248, 249,
	249, 249, 249, 249, 249, 249, 250, 250,
	250, 250, 250, 250, 250, 251, 251, 251,
	251, 251, 251, 251, 252, 252, 252, 252,
	252, 252, 252, 253, 253, 253, 253, 253,
	253, 253, 254, 254, 254, 254, 254, 254,
	254, 255, 255, 255, 255, 255, 255, 255,
};
const uint8_t detex_division_by_5_table[1280] = {
	0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 2, 2, 2, 2, 2, 3,
	3, 3, 3, 3, 4, 4, 4, 4,
	4, 5, 5, 5, 5, 5, 6, 6,
	6, 6, 6, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 9, 9, 9,
	9, 9, 10, 10, 10, 10, 10, 11,
	11, 11, 11, 11, 12, 12, 12, 12,
	12, 13, 13, 13, 13, 13, 14, 14,
	14, 14, 14, 15, 15, 15, 15, 15,
	16, 16, 16, 16, 16, 17, 17, 17,
	17, 17, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 20, 20, 20, 20,
	20, 21, 21, 21, 21, 21, 22, 22,
	22, 22, 22, 23, 23, 23, 23, 23,
	24, 24, 24, 24, 24, 25, 25, 25,
	25, 25, 26, 26, 26, 26, 26, 27,
	27, 27, 27, 27, 28, 28, 28, 28,
	28, 29, 29, 29, 29, 29, 30, 30,
	30, 30, 30, 31, 31, 31, 31, 31,
	32, 32, 32, 32, 32, 33, 33, 33,
	33, 33, 34, 34, 34, 34, 34, 35,
	35, 35, 35, 35, 36, 36, 36, 36,
	36, 37, 37, 37, 37, 37, 38, 38,
	38, 38, 38, 39, 39, 39, 39, 39,
	40, 40, 40, 40, 40, 41, 41, 41,
	41, 41, 42, 42, 42, 42, 42, 43,
	43, 43, 43, 43, 44, 44, 44, 44,
	44, 45, 45, 45, 45, 45, 46, 46,
	46, 46, 46, 47, 47, 47, 47, 47,
	48, 48, 48, 48, 48, 49, 49, 49,
	49, 49, 50, 50, 50, 50, 50, 51,
	51, 51, 51, 51, 52, 52, 52, 52,
	52, 53, 53, 53, 53, 53, 54, 54,
	54, 54, 54, 55, 55, 55, 55, 55,
	56, 56, 56, 56, 56, 57, 57, 57,
	57, 57, 58, 58, 58, 58, 58, 59,
	59, 59, 59, 59, 60, 60, 60, 60,
	60, 61, 61, 61, 61, 61, 62, 62,
	62, 62, 62, 63, 63, 63, 63, 63,
	64, 64, 64, 64, 64, 65, 65, 65,
	65, 65, 66, 66, 66, 66, 66, 67,
	67, 67, 67, 67, 68, 68, 68, 68,
	68, 69, 69, 69, 69, 69, 70, 70,
	70, 70, 70, 71, 71, 71, 71, 71,
	72, 72, 72, 72, 72, 73, 73, 73,
	73, 73, 74, 74, 74, 74, 74, 75,
	75, 75, 75, 75, 76, 76, 76, 76,
	76, 77, 77, 77, 77, 77, 78, 78,
	78, 78, 78, 79, 79, 79, 79, 79,
	80, 80, 80, 80, 80, 81, 81, 81,
	81, 81, 82, 82, 82, 82, 82, 83,
	83, 83, 83, 83, 84, 84, 84, 84,
	84, 85, 85, 85, 85, 85, 86, 86,
	86, 86, 86, 87, 87, 87, 87, 87,
	88, 88, 88, 88, 88, 89, 89, 89,
	89, 89, 90, 90, 90, 90, 90, 91,
	91, 91, 91, 91, 92, 92, 92, 92,
	92, 93, 93, 93, 93, 93, 94, 94,
	94, 94, 94, 95, 95, 95, 95, 95,
	96, 96, 96, 96, 96, 97, 97, 97,
	97, 97, 98, 98, 98, 98, 98, 99,
	99, 99, 99, 99, 100, 100, 100, 100,
	100, 101, 101, 101, 101, 101, 102, 102,
	102, 102, 102, 103, 103, 103, 103, 103,
	104, 104, 104, 104, 104, 105, 105, 105,
	105, 105, 106, 106, 106, 106, 106, 107,
	107, 107, 107, 107, 108, 108, 108, 108,
	108, 109, 109, 109, 109, 109, 110, 110,
	110, 110, 110, 111, 111, 111, 111, 111,
	112, 112, 112, 112, 112, 113, 113, 113,
	113, 113, 114, 114, 114, 114, 114, 115,
	115, 115, 115, 115, 116, 116, 116, 116,
	116, 117, 117, 117, 117, 117, 118, 118,
	118, 118, 118, 119, 119, 119, 119, 119,
	120, 120, 120, 120, 120, 121, 121, 121,
	121, 121, 122, 122, 122, 122, 122, 123,
	123, 123, 123, 123, 124, 124, 124, 124,
	124, 125, 125, 125, 125, 125, 126, 126,
	126, 126, 126, 127, 127, 127, 127, 127,
	128, 128, 128, 128, 128, 129, 129, 129,
	129, 129, 130, 130, 130, 130, 130, 131,
	131, 131, 131, 131, 132, 132, 132, 132,
	132, 133, 133, 133, 133, 133, 134, 134,
	134, 134, 134, 135, 135, 135, 135, 135,
	136, 136, 136, 136, 136, 137, 137, 137,
	137, 137, 138, 138, 138, 138, 138, 139,
	139, 139, 139, 139, 140, 140, 140, 140,
	140, 141, 141, 141, 141, 141, 142, 142,
	142, 142, 142, 143, 143, 143, 143, 143,
	144, 144, 144, 144, 144, 145, 145, 145,
	145, 145, 146, 146, 146, 146, 146, 147,
	147, 147, 147, 147, 148, 148, 148, 148,
	148, 149, 149, 149, 149, 149, 150, 150,
	150, 150, 150, 151, 151, 151, 151, 151,
	152, 152, 152, 152, 152, 153, 153, 153,
	153, 153, 154, 154, 154, 154, 154, 155,
	155, 155, 155, 155, 156, 156, 156, 156,
	156, 157, 157, 157, 157, 157, 158, 158,
	158, 158, 158, 159, 159, 159, 159, 159,
	160, 160, 160, 160, 160, 161, 161, 161,
	161, 161, 162, 162, 162, 162, 162, 163,
	163, 163, 163, 163, 164, 164, 164, 164,
	164, 165, 165, 165, 165, 165, 166, 166,
	166, 166, 166, 167, 167, 167, 167, 167,
	168, 168, 168, 168, 168, 169, 169, 169,
	169, 169, 170, 170, 170, 170, 170, 171,
	171, 171, 171, 171, 172, 172, 172, 172,
	172, 173, 173, 173, 173, 173, 174, 174,
	174, 174, 174, 175, 175, 175, 175, 175,
	176, 176, 176, 176, 176, 177, 177, 177,
	177, 177, 178, 178, 178, 178, 178, 179,
	179, 179, 179, 179, 180, 180, 180, 180,
	180, 181, 181, 181, 181, 181, 182, 182,
	182, 182, 182, 183, 183, 183, 183, 183,
	184, 184, 184, 184, 184, 185, 185, 185,
	185, 185, 186, 186, 186, 186, 186, 187,
	187, 187, 187, 187, 188, 188, 188, 188,
	188, 189, 189, 189, 189, 189, 190, 190,
	190, 190, 190, 191, 191, 191, 191, 191,
	192, 192, 192, 192, 192, 193, 193, 193,
	193, 193, 194, 194, 194, 194, 194, 195,
	195, 195, 195, 195, 196, 196, 196, 196,
	196, 197, 197, 197, 197, 197, 198, 198,
	198, 198, 198, 199, 199, 199, 199, 199,
	200, 200, 200, 200, 200, 201, 201, 201,
	201, 201, 202, 202, 202, 202, 202, 203,
	203, 203, 203, 203, 204, 204, 204, 204,
	204, 205, 205, 205, 205, 205, 206, 206,
	206, 206, 206, 207, 207, 207, 207, 207,
	208, 208, 208, 208, 208, 209, 209, 209,
	209, 209, 210, 210, 210, 210, 210, 211,
	211, 211, 211, 211, 212, 212, 212, 212,
	212, 213, 213, 213, 213, 213, 214, 214,
	214, 214, 214, 215, 215, 215, 215, 215,
	216, 216, 216, 216, 216, 217, 217, 217,
	217, 217, 218, 218, 218, 218, 218, 219,
	219, 219, 219, 219, 220, 220, 220, 220,
	220, 221, 221, 221, 221, 221, 222, 222,
	222, 222, 222, 223, 223, 223, 223, 223,
	224, 224, 224, 224, 224, 225, 225, 225,
	225, 225, 226, 226, 226, 226, 226, 227,
	227, 227, 227, 227, 228, 228, 228, 228,
	228, 229, 229, 229, 229, 229, 230, 230,
	230, 230, 230, 231, 231, 231, 231, 231,
	232, 232, 232, 232, 232, 233, 233, 233,
	233, 233, 234, 234, 234, 234, 234, 235,
	235, 235, 235, 235, 236, 236, 236, 236,
	236, 237, 237, 237, 237, 237, 238, 238,
	238, 238, 238, 239, 239, 239, 239, 239,
	240, 240, 240, 240, 240, 241, 241, 241,
	241, 241, 242, 242, 242, 242, 242, 243,
	243, 243, 243, 243, 244, 244, 244, 244,
	244, 245, 245, 245, 245, 245, 246, 246,
	246, 246, 246, 247, 247, 247, 247, 247,
	248, 248, 248, 248, 248, 249, 249, 249,
	249, 249, 250, 250, 250, 250, 250, 251,
	251, 251, 251, 251, 252, 252, 252, 252,
	252, 253, 253, 253, 253, 253, 254, 254,
	254, 254, 254, 255, 255, 255, 255, 255,
};
// ---------------------------------------------------------------------------------------------
static uint32_t detexDivide0To1791By7(uint32_t value) { return detex_division_by_7_table[value]; }
static uint32_t detexDivide0To1279By5(uint32_t value) { return detex_division_by_5_table[value]; }
// ---------------------------------------------------------------------------------------------
void DecodeBlockRGTC(const uint8_t* bitstring, int shift, int offset, uint8_t* pixel_buffer)
{
	// LSBFirst byte order only.
	uint64_t bits = (*(uint64_t *)&bitstring[0]) >> 16;
	int lum0 = bitstring[0];
	int lum1 = bitstring[1];
	for (int i = 0; i < 16; i++)
	{
		int control_code = bits & 0x7;
		uint8_t output;
		if (lum0 > lum1)
			switch (control_code)
			{
			case 0: output = lum0; break;
			case 1: output = lum1; break;
			case 2: output = detexDivide0To1791By7(6 * lum0 + lum1); break;
			case 3: output = detexDivide0To1791By7(5 * lum0 + 2 * lum1); break;
			case 4: output = detexDivide0To1791By7(4 * lum0 + 3 * lum1); break;
			case 5: output = detexDivide0To1791By7(3 * lum0 + 4 * lum1); break;
			case 6: output = detexDivide0To1791By7(2 * lum0 + 5 * lum1); break;
			case 7: output = detexDivide0To1791By7(lum0 + 6 * lum1); break;
			}
		else
			switch (control_code)
			{
			case 0: output = lum0; break;
			case 1: output = lum1; break;
			case 2: output = detexDivide0To1279By5(4 * lum0 + lum1); break;
			case 3: output = detexDivide0To1279By5(3 * lum0 + 2 * lum1); break;
			case 4: output = detexDivide0To1279By5(2 * lum0 + 3 * lum1); break;
			case 5: output = detexDivide0To1279By5(lum0 + 4 * lum1); break;
			case 6: output = 0; break;
			case 7: output = 0xFF; break;
			}
		pixel_buffer[(i << shift) + offset] = output;
		bits >>= 3;
	}
}
// ---------------------------------------------------------------------------------------------
void detexDecompressBlockRGTC2(const uint8_t* bitstring, uint8_t* pixel_buffer)
{
	DecodeBlockRGTC(bitstring, 1, 0, pixel_buffer);
	DecodeBlockRGTC(&bitstring[8], 1, 1, pixel_buffer);
}
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

void NEW_DECOMPRESS_DXT1(uint32_t x, uint32_t y, uint32_t stride, const uint8_t* blockStorage, unsigned char* image)
{
	//static const uint8_t const_alpha[] =
	static const uint8_t alphaValues[] =
	{
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255
	};

	// DecompressBlockBC1Internal(blockStorage, image + x * sizeof(uint32_t) + (y * stride), stride, const_alpha);

	unsigned char* output = image + x * sizeof(uint32_t) + (y * stride);
	{
		uint32_t temp, code;

		uint16_t color0, color1;
		uint8_t r0, g0, b0, r1, g1, b1;

		int i, j;

		color0 = *(const uint16_t*)(blockStorage);
		color1 = *(const uint16_t*)(blockStorage + 2);

		temp = (color0 >> 11) * 255 + 16;
		r0 = (uint8_t)((temp / 32 + temp) / 32);
		temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
		g0 = (uint8_t)((temp / 64 + temp) / 64);
		temp = (color0 & 0x001F) * 255 + 16;
		b0 = (uint8_t)((temp / 32 + temp) / 32);

		temp = (color1 >> 11) * 255 + 16;
		r1 = (uint8_t)((temp / 32 + temp) / 32);
		temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
		g1 = (uint8_t)((temp / 64 + temp) / 64);
		temp = (color1 & 0x001F) * 255 + 16;
		b1 = (uint8_t)((temp / 32 + temp) / 32);

		code = *(const uint32_t*)(blockStorage + 4);

		if (color0 > color1)
		{
			for (j = 0; j < 4; ++j)
			{
				for (i = 0; i < 4; ++i)
				{
					uint32_t finalColor, positionCode;
					uint8_t alpha;

					alpha = alphaValues[j * 4 + i];

					finalColor = 0;
					positionCode = (code >> 2 * (4 * j + i)) & 0x03;

					switch (positionCode)
					{
					case 0:
						finalColor = PackRGBA(r0, g0, b0, alpha);
						break;
					case 1:
						finalColor = PackRGBA(r1, g1, b1, alpha);
						break;
					case 2:
						finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, alpha);
						break;
					case 3:
						finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, alpha);
						break;
					}

					//*(uint32_t*)(output + j * stride + i * sizeof(uint32_t)) = finalColor;
					((unsigned long*)image)[(y + j)*stride + (x + i)] = finalColor;
				}
			}
		}
		else
		{
			for (j = 0; j < 4; ++j)
			{
				for (i = 0; i < 4; ++i)
				{
					uint32_t finalColor, positionCode;
					uint8_t alpha;

					alpha = alphaValues[j * 4 + i];

					finalColor = 0;
					positionCode = (code >> 2 * (4 * j + i)) & 0x03;

					switch (positionCode)
					{
					case 0:
						finalColor = PackRGBA(r0, g0, b0, alpha);
						break;
					case 1:
						finalColor = PackRGBA(r1, g1, b1, alpha);
						break;
					case 2:
						finalColor = PackRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, alpha);
						break;
					case 3:
						finalColor = PackRGBA(0, 0, 0, alpha);
						break;
					}

					//*(uint32_t*)(output + j * stride + i * sizeof(uint32_t)) = finalColor;

					//if (x + i < width)
					((unsigned long*)image)[(y + j)*stride + (x + i)] = finalColor;
				}
			}
		}
	}
}

void OLD_DECOMPRESS_DXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, void *img)
{
	unsigned long *image = (unsigned long*)img;
	//unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage);
	//unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 2);

	uint16_t color0, color1;
	color0 = *(const uint16_t*)(blockStorage);
	color1 = *(const uint16_t*)(blockStorage + 2);

	unsigned long temp;

	uint8_t r0, g0, b0, r1, g1, b1;

	temp = (color0 >> 11) * 255 + 16;
	r0 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	g0 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	b0 = (uint8_t)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	r1 = (uint8_t)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	g1 = (uint8_t)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	b1 = (uint8_t)((temp / 32 + temp) / 32);

	unsigned long code = *(const uint32_t*)(blockStorage + 4);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			unsigned long finalColor = 0;
			unsigned char positionCode = (code >> 2 * (4 * j + i)) & 0x03;

			if (color0 > color1)
			{
				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, 255);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, 255);
					break;
				case 2:
					finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, 255);
					break;
				case 3:
					finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, 255);
					break;
				}
			}
			else
			{
				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, 255);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, 255);
					break;
				case 2:
					finalColor = PackRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, 255);
					break;
				case 3:
					finalColor = PackRGBA(0, 0, 0, 255);
					break;
				}
			}

			//unsigned char* output = (unsigned char*)image + x * sizeof(uint32_t) + (y * width);
			//*(uint32_t*)(output + j * width + i * sizeof(uint32_t)) = finalColor;

			if (x + i < width)
				image[(y + j)*width + (x + i)] = finalColor;
		}
	}
}

void BlockDecompressImageBC1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image)
{
	unsigned long blockCountX = (width + 3) / 4;
	unsigned long blockCountY = (height + 3) / 4;
	unsigned long blockWidth = (width < 4) ? width : 4;
	unsigned long blockHeight = (height < 4) ? height : 4;

	for (unsigned long j = 0; j < blockCountY; j++)
	{
		for (unsigned long i = 0; i < blockCountX; i++)
			//	DecompressBlockBC1(i * 4, j * 4, width, blockStorage + i * 8, image);
			NEW_DECOMPRESS_DXT1(i * 4, j * 4, width, blockStorage + i * 8, image);

		blockStorage += blockCountX * 8;
	}
}
void BlockDecompressImageBC3(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image)
{
	unsigned long blockCountX = (width + 3) / 4;
	unsigned long blockCountY = (height + 3) / 4;
	unsigned long blockWidth = (width < 4) ? width : 4;
	unsigned long blockHeight = (height < 4) ? height : 4;

	for (unsigned long j = 0; j < blockCountY; j++)
	{
		for (unsigned long i = 0; i < blockCountX; i++)
			DecompressBlockBC3(i * 4, j * 4, width, blockStorage + i * 16, image);
		blockStorage += blockCountX * 16;
	}
}
void BlockDecompressImageBC5(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image)
{
	unsigned long blockCountX = (width + 3) / 4;
	unsigned long blockCountY = (height + 3) / 4;
	unsigned long blockWidth = (width < 4) ? width : 4;
	unsigned long blockHeight = (height < 4) ? height : 4;

	crntImgPtr = (uint32_t*)image;
	crntImgPtrEnd = &((uint32_t*)image)[width * height];

	uint8_t* input = (uint8_t*)blockStorage;
	uint8_t* output = (uint8_t*)image;

	for (unsigned long j = 0; j < blockCountY; j++)
	{
		for (unsigned long i = 0; i < blockCountX; i++)
		{ 
			// DecompressBlockBC3(i * 4, j * 4, width, blockStorage + i * 16, image);
			DecompressBlockBC5(i * 4, j * 4, width, BC5_UNORM, blockStorage + i * 16, image);
		
			//detexDecompressBlockRGTC2(input, output);
			//input += sizeof(16);
			//input += blockCountX * 16;

			// output += 16;
			// input += 16;
		}

		blockStorage += blockCountX * 16;
	}

	printf("remaining image bytes: %d\n", (int64_t)crntImgPtrEnd - (int64_t)crntImgPtr);
}
