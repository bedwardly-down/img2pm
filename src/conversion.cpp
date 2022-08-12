#include "conversion.hpp"

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

  nSprite = 0;

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
