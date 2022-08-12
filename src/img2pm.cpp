// img2pm
// Libraries required: DevIL.lib ILU.lib ILUT.lib
// download at: http://openil.sourceforge.net/

// Update 02-06-2022 (Brandon D <bedwardly-down>
// download latest source for compatibility with newer gcc/visual studio: https://github.com/DentonW/DevIL
// compile as per Readme.cmake

#define _CRT_SECURE_NO_WARNINGS

#define MAX_INFILES     32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <ctype.h>

#include "conversion.hpp"

// Indexes in datTile array
#define MASK_TL 0
#define MASK_BL 1
#define DRAW_TL 2
#define DRAW_BL 3
#define MASK_TR 4
#define MASK_BR 5
#define DRAW_TR 6
#define DRAW_BR 7

int   nSprite;
bool  bHeader;

char* fileNameOut;        // output file
char* fileBase = "0x010000"; // base address
bool bHasSprites;
bool bHasTiles;
int  nShadesTiles;

// Builds an 8x8 tile from source image
// This will build the draw and mask information for the tile
void buildTile(ILubyte *px, unsigned char* datDraw, unsigned char* datMask, ILint width, int abs_x, int abs_y, int nshades, int frame)
{
  int x,y;
  int grey;
  
  // clear the arrays
  memset(datDraw, 0, 8);
  memset(datMask, 0, 8);

  // Build 8x8 pixels draw tile
  for(y=0;y<8;y++)
    for(x=0;x<8;x++) {
      // Average pixel information to get grey value of pixel
      grey = ((int)px[x*4+y*4*width] + (int)px[x*4+y*4*width+1] + (int)px[x*4+y*4*width+2])/3;
      // build pixel
      if(((x+abs_x+(y+abs_y)+frame) % (nshades-1)) >= ((grey*nshades)/256)) 
        datDraw[x] |= 1<<y; // set the pixel
      // Check alpha channel
      if(px[x*4+y*4*width+3] < 128)
        datMask[x] |= 1<<y; // make pixel transparent
    }
}

char* formatFileName(char *fileName, bool toUpper)
{
  char* pchr;
  static char OutBuf[128];
  char* pos;
  strcpy(OutBuf, fileName);

  // End the string at last dot (typically the file extension)
  if((pchr = strrchr(OutBuf,'.')) != NULL)
    *pchr = 0;

  // replace all dots with _
  while((pchr = strchr(OutBuf,'.')) != NULL)
    *pchr = '_';

  // find beginning of string (skip all / and \)
  pchr = OutBuf;
  pos = strrchr(OutBuf,'/')+1;
  if(pos>pchr) pchr = pos;
  pos = strrchr(OutBuf,'\\')+1;
  if(pos>pchr) pchr = pos;
  pos = pchr;

  if(toUpper) {
    while(*pchr) {
      *pchr = toupper(*pchr);
      pchr++;
    }
  }
  return pos;
}

bool ConvertSprites(tFile *inFile)
{
  ILuint ImageID;
  ILboolean success;
  ILubyte *data;
  ILint width;

  int nSpritesX, nSpritesY, nImages;
  unsigned char datTile[8][8];
  unsigned char* pd; // data pointer
  int x, y, frame, image;

  // Generate & Bind image
  ilGenImages(1, &ImageID);
  ilBindImage(ImageID);

  ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

  // Load the file
  success = ilLoadImage((ILstring)inFile->fileName); /* Loading of image "image.jpg" */
  if(success != IL_TRUE) {
    printf("ERROR (%s): Couldn't load input file!", inFile->fileName);
    return false;
  }

  // Flip image if origin is wrong
  if(ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_UPPER_LEFT) {
    iluFlipImage();
  }

  // Check image sizes
  if(ilGetInteger(IL_IMAGE_WIDTH) & 0xF) {
    printf("ERROR (%s): Image width is not an exact multiple of 16 pixels!", inFile->fileName);
    return false;
  }
  if(ilGetInteger(IL_IMAGE_HEIGHT) & 0xF) {
    printf("ERROR (%s): Image height is not an exact multiple of 16 pixels!", inFile->fileName);
    return false;
  }

  // Number of sprites inside this file
  nSpritesX = (ilGetInteger(IL_IMAGE_WIDTH)/16);
  nSpritesY = (ilGetInteger(IL_IMAGE_HEIGHT)/16);
  // Number of images in this file (for animations)
  nImages = ilGetInteger(IL_NUM_IMAGES)+1;
  
  // File is loaded & checked, start the conversion
  width = ilGetInteger(IL_IMAGE_WIDTH);

  // allocate memory for all sprites
  inFile->length = nSpritesX*nSpritesY*nImages*8*8*(inFile->shades-1);
  inFile->data = (unsigned char*)malloc(inFile->length);
  // setup data pointer
  pd = inFile->data;

  // cycle throught all images
  for(image=0;image<nImages;image++) {
    // select active image
    if(ilActiveImage(image)!=IL_TRUE) {
      printf("ERROR (%s): Could not select image #%d!", inFile->fileName, image);
      return false;
    }

    // Convert image into a known format
    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if(success != IL_TRUE) {
      printf("ERROR (%s): Couldn't convert input file to desired format!", inFile->fileName);
      return false;
    }

    // Get pointer to image data
    data = ilGetData();
    if(data == NULL) {
      printf("ERROR (%s): Couldn't retrieve pointer to bitmap data!", inFile->fileName);
      return false;
    }
  
    for(y=0;y<nSpritesY;y++) {
        for(x=0;x<nSpritesX;x++) {
          for(frame=1;frame<=(inFile->shades-1);frame++) {
            // build top left tile (draw and mask)
            buildTile(data+16*4*x, datTile[DRAW_TL], datTile[MASK_TL], width, 0, 0, inFile->shades, frame);
            // build top right tile (draw and mask)
            buildTile(data+8*4+16*4*x, datTile[DRAW_TR], datTile[MASK_TR], width, 8, 0, inFile->shades, frame);
            // build bottom left tile (draw and mask)
            buildTile(data+8*4*width+16*4*x, datTile[DRAW_BL], datTile[MASK_BL], width, 0, 8, inFile->shades, frame);
            // build bottom right tile (draw and mask)
            buildTile(data+8*4*width+8*4+16*4*x, datTile[DRAW_BR], datTile[MASK_BR], width, 8, 8, inFile->shades, frame);
            memcpy(pd,datTile,8*8); pd+=8*8;
            nSprite++;
          }
        }
        data += width*4*16; // skip 16 pixels in Y direction
    }
  }

  ilDeleteImages(1, &ImageID);
  return true;
}

bool ConvertTiles(tFile *inFile)
{
  ILuint ImageID;
  ILboolean success;
  ILubyte *data, *img_data;
  ILint width;

  int nTilesX, nTilesY, nImages;
  unsigned char datMask[8]; // not needed
  unsigned char* pd; // data pointer
  int x, y, frame, image;

  // Generate & Bind image
  ilGenImages(1, &ImageID);
  ilBindImage(ImageID);

  // Load the file
  success = ilLoadImage((ILstring)inFile->fileName); /* Loading of image "image.jpg" */
  if(success != IL_TRUE) {
    printf("ERROR (%s): Couldn't load input file!", inFile->fileName);
    return false;
  }

  // Flip image if origin is wrong
  if(ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_UPPER_LEFT) {
    iluFlipImage();
  }

  // Check image sizes
  if(ilGetInteger(IL_IMAGE_WIDTH) & 0x7) {
    printf("ERROR (%s): Image width is not an exact multiple of 8 pixels!", inFile->fileName);
    return false;
  }
  if(ilGetInteger(IL_IMAGE_HEIGHT) & 0x7) {
    printf("ERROR (%s): Image height is not an exact multiple of 8 pixels!", inFile->fileName);
    return false;
  }

  // Number of sprites inside this file
  nTilesX = (ilGetInteger(IL_IMAGE_WIDTH)/8);
  nTilesY = (ilGetInteger(IL_IMAGE_HEIGHT)/8);
  // Number of images in this file (for animations)
  nImages = ilGetInteger(IL_NUM_IMAGES)+1;
  
  // File is loaded & checked, start the conversion
  width = ilGetInteger(IL_IMAGE_WIDTH);

  // allocate memory for all sprites
  inFile->length = nTilesX*nTilesY*nImages*8*(inFile->shades-1);
  inFile->data = (unsigned char*)malloc(inFile->length);
  // setup data pointer
  pd = inFile->data;

  // cycle through all images
  for(image=0;image<nImages;image++) {
    // select active image
    if(ilActiveImage(image)!=IL_TRUE) {
      printf("ERROR (%s): Could not select image #%d!", inFile->fileName, image);
      return false;
    }

    // Convert image into a known format
    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if(success != IL_TRUE) {
      printf("ERROR (%s): Couldn't convert input file to desired format!", inFile->fileName);
      return false;
    }

    // Get pointer to image data
    img_data = ilGetData();
    if(img_data == NULL) {
      printf("ERROR (%s): Couldn't retrieve pointer to bitmap data!", inFile->fileName);
      return false;
    }

    for(frame=1;frame<=(inFile->shades-1);frame++) {
      data = img_data;
      for(y=0;y<nTilesY;y++) {
        for(x=0;x<nTilesX;x++) {
          // build tile
          buildTile(data+4*8*x, pd, datMask, width, x*8, y*8, inFile->shades, frame);
          pd+=8;
        }
        data += width*4*8; // skip 8 pixels in Y direction
      }
      
    }
    image++; // next image
  }

  ilDeleteImages(1, &ImageID);
  return true;
}


int main(int argc, char* argv[])
{
  char *strStart, *strEnd;
  char AppName[64];
  FILE *fileOut;
  int nFile, i, j;
  unsigned char *pd;
  bool btmp;
  
  printf("img2pm V1.1 by bedwardly-down\n");
  printf("updated from v1.0 by Lupin\n");
  printf("http://lupin.shizzle.it\n");
  printf("-----------------------\n\n");

  // find start of string
  strStart = argv[0];
  strEnd = strrchr(argv[0],'\\')+1;
  if(strEnd>strStart) strStart = strEnd;
  strEnd = strrchr(argv[0],'/')+1;
  if(strEnd>strStart) strStart = strEnd;
  // now find end of string
  strEnd=strrchr(argv[0],'.');
  if(!strEnd || (strEnd<strStart)) strEnd=strStart+strlen(strStart);
  i=strEnd-strStart;
  strncpy(AppName,strStart,i);
  AppName[i]='\0';

  if(argc<3) {
    printf("Usage: %s -o [file] [-g files] [-bw files] [-b address]\n", AppName);
    printf("Commands:\n");
    printf("-o file       Output file\n");
    printf("              Extension .h outputs header file\n");
    printf("-b address    Base address to use (Default: 0x010000)\n"); // Taken from https://github.com/logicplace/pokemini-img2c
    printf("-s2 files     Convert 2 shade sprites (1 frame)\n");
    printf("-s3 files     Convert 3 shade sprites (2 frames)\n");
    printf("-t2 files     Convert 2 shade tiles (1 frame)\n");
    printf("-t3 files     Convert 3 shade tiles (2 frames)\n");
    printf("-t4 files     Convert 3 shade tiles (3 frames)\n");
    printf("-t5 files     Convert 3 shade tiles (4 frames)\n");
    printf("-t6 files     Convert 6 shade tiles (5 frames)\n");
    printf("\n\rExample:\n");
    printf("%s -o sprites.h -g anim.gif -bw sprites.bmp\n", AppName);
    printf("This would convert the animation from anim.gif to greyscale sprites and all\n");
    printf("sprites in sprites.bmp to b&w sprites. The output will be written to sprites.h\n");
    printf("\n\rPress return key to continue...\n");
    getchar();
    return -1;
  }

  bHasSprites = false;
  bHasTiles = false;
  nFile = 0;
  for(i=1;i<argc;i++) {
    if(strcmp(argv[i],"-o")==0) {
      // output file option
      i++;
      if(strrchr(argv[i],'.')[1] == 'h')
        bHeader = true;
      else
        bHeader = false;
      fileNameOut = argv[i];

    } else if(strcmp(argv[i],"-b")==0) {
      i++;
      fileBase = argv[i];

    } else if(argv[i][0] == '-') {
      j = argv[i][2]-'0'; // get number of shades
      if(argv[i][1] == 's') {
        btmp = true;
        bHasSprites = true;
      } else if(argv[i][1] == 't') {
        if(bHasTiles) {
          printf("ERROR: Can't convert more than one tileset per call!\n");
          return -1;
        }
        nShadesTiles = j;
        btmp = false;
        bHasTiles = true;
      } else {
        // Unknown option
        printf("ERROR: Unknown option!\n");
        getchar();
        return -1;
      }

      // build the file list
      if((j<2)||(j>6)) {
        printf("ERROR: Unknown option!\n");
        getchar();
        return -1;
      }
      i++;
      while(argv[i]) {
        if(argv[i][0] != '-') {
          files[nFile].base = fileBase;
          files[nFile].sprite = btmp;
          files[nFile].shades = j;
          files[nFile].fileName = argv[i];
          // generate symbolic name and allocate space for it in tFile structure
          strStart = formatFileName(argv[i], true);
          files[nFile].fileNameSym = (char*)malloc(strlen(strStart)+1);
          strcpy(files[nFile].fileNameSym, formatFileName(argv[i], true));
          i++; nFile++;
        } else {
          i--;
          break;
        }
      }

    }
  }
  files[nFile].fileName = NULL; // fileName = NULL indicates end of file list

  /* Convert image files */
  ilInit(); /* Initialization of DevIL */
  nFile = 0;
  nSprite = 0;
  while(files[nFile].fileName!=NULL) {
    printf("Converting %s to %d shades %s...\n", files[nFile].fileName, files[nFile].shades, files[nFile].sprite?"sprites":"tiles");
    if(files[nFile].sprite)
      btmp = ConvertSprites(&files[nFile]);
    else
      btmp = ConvertTiles(&files[nFile]);
    
    if(btmp == false) return -1;
    nFile++; // next file
  }

  // write Header or Binary file
  fileOut = fopen(fileNameOut, "wb");
  if(bHeader) {
    // Output header file
    fprintf(fileOut,"#include \"pm.h\"\n");
    fprintf(fileOut,"#include <stdint.h>\n\n");
    if(bHasSprites) {
      fprintf(fileOut,"// Sprites\n");
      nFile = 0; i=0;
      while(files[nFile].fileName!=NULL) {
        if(!files[nFile].sprite) {
          nFile++; continue;
        }
        fprintf(fileOut,"#define %s_ID %*s %d\n",files[nFile].fileNameSym,(32-3)-strlen(files[nFile].fileNameSym),"",i);
        fprintf(fileOut,"#define %s_FRAMES %*s %d\n",files[nFile].fileNameSym,(32-7)-strlen(files[nFile].fileNameSym),"",(files[nFile].length/(8*8)) / (files[nFile].shades-1));
        i += files[nFile].length / (8*8);
        nFile++;
      }
      fprintf(fileOut, "\n");
    }

    if(bHasTiles) {
      fprintf(fileOut,"// Tiles\n");
      nFile = 0; i=0;
      while(files[nFile].fileName!=NULL) {
        if(files[nFile].sprite) {
          nFile++; continue;
        }
        fprintf(fileOut,"#define %s_OFFS %*s %d\n",files[nFile].fileNameSym,(32-5)-strlen(files[nFile].fileNameSym),"",i);
        fprintf(fileOut,"#define %s_FRAMES %*s %d\n",files[nFile].fileNameSym,(32-7)-strlen(files[nFile].fileNameSym),"",files[nFile].length/768 / (nShadesTiles-1));
        fprintf(fileOut,"#define %s_TILES %*s %d\n",files[nFile].fileNameSym,(32-6)-strlen(files[nFile].fileNameSym),"",(files[nFile].length/8) / (nShadesTiles-1));
        i += files[nFile].length;
        nFile++;
      }
      fprintf(fileOut, "\n");
    }
    
    if(bHasSprites) {
      // output sprite data
      fprintf(fileOut,"const uint8_t _rom %s%s[] _at(%s) = {\n", formatFileName(fileNameOut, false), bHasTiles?"_spr":"", fileBase);
      // cycle through all files and output the converted sprite data
      nFile = 0;
      while(files[nFile].fileName!=NULL) {
        if(!files[nFile].sprite) {
          nFile++; continue;
        }
        fprintf(fileOut, "  // %s\n", files[nFile].fileName);
        pd = files[nFile].data;
        for(j=0;j<files[nFile].length/64;j++) {
          // Output one sprite 
          for(i=0;i<8;i++) {
            fprintf(fileOut, "  0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,\n", pd[0], pd[1], pd[2], pd[3], pd[4], pd[5], pd[6], pd[7]); pd+=8;
          }
          fprintf(fileOut,"\n");
        }
        fprintf(fileOut, "\n");
        nFile++; // next file
      }
      fprintf(fileOut,"};\n\n");
    }

    if(bHasTiles) {
      // output tile data
      for(i=0;i<(nShadesTiles-1);i++) {
        fprintf(fileOut,"const uint8_t _rom %s%s_frame%d[] _at(%s) = {\n", formatFileName(fileNameOut, false), bHasSprites?"_tile":"",i,fileBase);
        // cycle through all files and output the converted tile data
        nFile = 0;
        while(files[nFile].fileName!=NULL) {
          if(files[nFile].sprite) {
            nFile++; continue;
          }
          fprintf(fileOut, "  // %s\n", files[nFile].fileName);
          pd = files[nFile].data+i*(files[nFile].length/(nShadesTiles-1));
          for(j=0;j<files[nFile].length/8/(nShadesTiles-1);j++) {
            fprintf(fileOut, "  0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,\n", pd[0], pd[1], pd[2], pd[3], pd[4], pd[5], pd[6], pd[7]); pd+=8;
          }
          fprintf(fileOut, "\n");
          nFile++; // next file
        }
        fprintf(fileOut,"};\n\n");
      }
    }
  } else {
    // Output raw binary
    if(bHasSprites) {
      // cycle through all files and output the converted sprite data
      nFile = 0;
      while(files[nFile].fileName!=NULL) {
        if(!files[nFile].sprite) {
          nFile++; continue;
        }
        fwrite(files[nFile].data, 1, files[nFile].length, fileOut);
        nFile++; // next file
      }
    }
    if(bHasTiles) {
      // output tile data
      for(i=0;i<(nShadesTiles-1);i++) {
        // cycle through all files and output the converted tile data
        nFile = 0;
        while(files[nFile].fileName!=NULL) {
          if(files[nFile].sprite) {
            nFile++; continue;
          }
          fwrite(files[nFile].data+i*(files[nFile].length/(nShadesTiles-1)), 1, files[nFile].length/(nShadesTiles-1), fileOut);
          nFile++; // next file
        }
      }
    }
  }

  // cleanup
  nFile = 0;
  while(files[nFile].fileName!=NULL) {
    if(files[nFile].data!=NULL) free(files[nFile].data);
    if(files[nFile].fileNameSym!=NULL) free(files[nFile].fileNameSym);
    nFile++;
  }

  fclose(fileOut);

	return 0;
}

