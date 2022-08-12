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
#include <ctype.h>

#include "conversion.hpp"

bool  bHeader;
int   nSprite;

char* fileNameOut;        // output file
char* fileBase = "0x010000"; // base address
bool bHasSprites;
bool bHasTiles;
int  nShadesTiles;
Conversion conversion;

tFile files[MAX_INFILES]; // input files

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
  while(files[nFile].fileName!=NULL) {
    printf("Converting %s to %d shades %s...\n", files[nFile].fileName, files[nFile].shades, files[nFile].sprite?"sprites":"tiles");
    if(files[nFile].sprite)
      btmp = conversion.ConvertSprites(&files[nFile]);
    else
      btmp = conversion.ConvertTiles(&files[nFile]);
    
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

