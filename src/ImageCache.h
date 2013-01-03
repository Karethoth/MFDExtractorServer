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



typedef struct
{
	WORD x;
	WORD y;
	PIXEL color;
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
	std::vector<PIXDIFF> diff;

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
	{ return &diff; }
	  
	std::vector<PIXDIFF*> *GetImage( AREA &area );
	std::vector<PIXDIFF*> *GetDiff( AREA &area );
};
