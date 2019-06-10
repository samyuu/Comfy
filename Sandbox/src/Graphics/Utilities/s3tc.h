#pragma once

// unsigned long PackRGBA(): Helper method that packs RGBA channels into a single 4 byte pixel.
//
// unsigned char r:     red channel.
// unsigned char g:     green channel.
// unsigned char b:     blue channel.
// unsigned char a:     alpha channel.

unsigned long PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.
//
// unsigned long x:                     x-coordinate of the first pixel in the block.
// unsigned long y:                     y-coordinate of the first pixel in the block.
// unsigned long width:                 width of the texture being decompressed.
// unsigned long height:                height of the texture being decompressed.
// const unsigned char *blockStorage:   pointer to the block to decompress.
// unsigned long *image:                pointer to image where the decompressed pixel data should be stored.

void DecompressBlockDXT1(
	unsigned long x, 
	unsigned long y, 
	unsigned long width, 
	const unsigned char *blockStorage, 
	unsigned long *image);

// void BlockDecompressImageDXT1(): Decompresses all the blocks of a DXT1 compressed texture and stores the resulting pixels in 'image'.
//
// unsigned long width:                 Texture width.
// unsigned long height:                Texture height.
// const unsigned char *blockStorage:   pointer to compressed DXT1 blocks.
// unsigned long *image:                pointer to the image where the decompressed pixels will be stored.

void BlockDecompressImageDXT1(
	unsigned long width,
	unsigned long height,
	const unsigned char *blockStorage,
	unsigned long *image);

// void DecompressBlockDXT5(): Decompresses one block of a DXT5 texture and stores the resulting pixels at the appropriate offset in 'image'.
//
// unsigned long x:                     x-coordinate of the first pixel in the block.
// unsigned long y:                     y-coordinate of the first pixel in the block.
// unsigned long width:                 width of the texture being decompressed.
// unsigned long height:                height of the texture being decompressed.
// const unsigned char *blockStorage:   pointer to the block to decompress.
// unsigned long *image:                pointer to image where the decompressed pixel data should be stored.

void DecompressBlockDXT5(
	unsigned long x, 
	unsigned long y, 
	unsigned long width, 
	const unsigned char *blockStorage, 
	unsigned long *image);

// void BlockDecompressImageDXT5(): Decompresses all the blocks of a DXT5 compressed texture and stores the resulting pixels in 'image'.
//
// unsigned long width:                 Texture width.
// unsigned long height:                Texture height.
// const unsigned char *blockStorage:   pointer to compressed DXT5 blocks.
// unsigned long *image:                pointer to the image where the decompressed pixels will be stored.

void BlockDecompressImageDXT5(
	unsigned long width, 
	unsigned long height, 
	const unsigned char *blockStorage, 
	unsigned long *image);