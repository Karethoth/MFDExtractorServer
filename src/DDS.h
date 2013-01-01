#pragma once

#include "stdafx.h"
#include <Windows.h>


enum DDS_FLAGS
{
	DDSD_CAPS   = 0x1,
	DDSD_HEIGHT = 0x2,
	DDSD_WIDTH  = 0x4,
	DDSD_PITCH  = 0x8,
	DDSD_PIXELFORMAT = 0x1000,
	DDSD_MIPMAPCOUNT = 0x20000,
	DDSD_LINEARSIZE  = 0x80000,
	DDSD_DEPTH       = 0x800000
};


enum DDS_CAPS
{
	DDSCAPS_COMPLEX = 0x8,
	DDSCAPS_MIPMAP  = 0x400000,
	DDSCAPS_TEXTURE = 0x1000
};


enum DDS_CAPS2
{
	DDSCAPS2_CUBEMAP           = 0x200,
	DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
	DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
	DDSCAPS2_VOLUME            = 0x200000
};


struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};


typedef struct {
	DWORD           dwMagic;
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;


extern size_t DDS_CalculateImageDataSize( DDS_HEADER *header );