#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <string>

#include "DDS.h"

#pragma pack(1)
typedef struct
{
	WORD  bfType;
	DWORD bfSize;
	WORD  bfRes1;
	WORD  bfRes2;
	DWORD bfOffset;
	DWORD biSize;
	DWORD biWidth;
	DWORD biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	DWORD biXPelsPerMeter;
	DWORD biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BMP_HEADER;


extern bool SaveDDSFromMemoryToBMP(	std::string filepath, DDS_HEADER *header, BYTE *data );
