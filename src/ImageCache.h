#pragma once
#define _WINSOCKAPI_
#include <Windows.h>
#include <vector>
#include "DDS.h"


typedef struct
{
	BYTE r;
	BYTE g;
	BYTE b;
} PIXEL;



typedef struct sPIXDIFF
{
	WORD x;
	WORD y;
	PIXEL color;

	sPIXDIFF()
	{
		x=0xffff;
		y=0xffff;
		color.r = 0x01;
		color.g = 0x02;
		color.b = 0x01;
	}
} PIXDIFF;



typedef struct
{
	WORD x;
	WORD y;
	WORD w;
	WORD h;
} AREA;



class ImageCache
{
private:
	std::vector<PIXDIFF> *cache;
	std::vector<PIXDIFF> *oldCache;
	std::vector<PIXDIFF> *diff;

	HANDLE hMapFileHeader;
	HANDLE hMapFileData;

	DDS_HEADER *textureHeader;
	BYTE       *textureData;

protected:
	bool OpenMemory();

public:
	ImageCache();
	~ImageCache();


	int Update();
	  
	std::vector<PIXDIFF>* GetImage()
	{ return cache; }

	std::vector<PIXDIFF>* GetDiff()
	{ return diff; }
	  
	int GetRGBData( AREA &area, std::vector<PIXEL> &dest );
	int GetImage( AREA &area, std::vector<PIXDIFF> &dest );
	int GetDiff( AREA &area, std::vector<PIXDIFF> &dest );
};
