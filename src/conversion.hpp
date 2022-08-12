#ifndef _CONVERSION_HPP
#define _CONVERSION_HPP

#include <stdlib.h>
#include <string.h>
#include <IL/il.h>
#include <IL/ilu.h>

#define MAX_INFILES     32

// Indexes in datTile array
#define MASK_TL 0
#define MASK_BL 1
#define DRAW_TL 2
#define DRAW_BL 3
#define MASK_TR 4
#define MASK_BR 5
#define DRAW_TR 6
#define DRAW_BR 7

struct tFile {
	char* fileName;
	char* fileNameSym;
    char* base;
	int length;
	int shades;
    bool sprite;
	unsigned char* data;
};

int   nSprite;
tFile files[MAX_INFILES]; // input files

void buildTile(ILubyte *px, unsigned char* datDraw, unsigned char* datMask, ILint width, int abs_x, int abs_y, int nshades, int frame);

bool ConvertSprites(tFile *inFile);

bool ConvertTiles(tFile *inFile);

#endif