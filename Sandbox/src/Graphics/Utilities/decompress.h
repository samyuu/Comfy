#pragma once

void BlockDecompressImageBC1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image);
void BlockDecompressImageBC3(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image);
void BlockDecompressImageBC5(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned char *image);
