#ifndef _CONVERSION_HPP
#define _CONVERSION_HPP

#define MAX_INFILES     32

struct tFile {
	char* fileName;
	char* fileNameSym;
    char* base;
	int length;
	int shades;
    bool sprite;
	unsigned char* data;
};

tFile files[MAX_INFILES]; // input files

#endif