#pragma once
#define _WINSOCKAPI_
#include <Windows.h>
#include <vector>


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

  public:
	  ImageCache();
	  ~ImageCache();

	  int Update();
	  
	  std::vector<PIXDIFF>* GetImage()
	  { return cache; }

	  std::vector<PIXDIFF>* GetDiff()
	  { return &diff; }
};
