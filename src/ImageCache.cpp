#include "stdafx.h"
#include "ImageCache.h"
#include "DDS.h"


TCHAR sharedMemoryAreaName[] = TEXT( "FalconTexturesSharedMemoryArea" );


ImageCache::ImageCache()
{
	cache = new std::vector<PIXDIFF>();
}



ImageCache::~ImageCache()
{
	cache->clear();
	delete cache;
	diff.clear();
}



int ImageCache::Update()
{
	DDS_HEADER *textureHeader;
	BYTE       *textureData;

	diff.clear();
	
	hMapFileHeader = OpenFileMapping(
					FILE_MAP_ALL_ACCESS,
					FALSE,
					sharedMemoryAreaName
				);               // name of mapping object
	if( hMapFileHeader == NULL )
	{
		_tprintf( TEXT("Could not open file mapping object (%d).\n"),
				GetLastError() );
		return 1;
	}

	hMapFileData = OpenFileMapping(
					FILE_MAP_ALL_ACCESS,
					FALSE,
					sharedMemoryAreaName
				);               // name of mapping object
	if( hMapFileData == NULL )
	{
		_tprintf( TEXT("Could not open file mapping object (%d).\n"),
				GetLastError() );
		return 1;
	}

	/* READ THE HEADER */
	textureHeader = (DDS_HEADER*)MapViewOfFile( hMapFileHeader,
												FILE_MAP_ALL_ACCESS,
												0,
												0,
												40
											);
	if( textureHeader == NULL )
	{
		_tprintf( TEXT("Couldn't fetch the header! (%d).\n"),
				GetLastError() );

		CloseHandle( hMapFileHeader );
		CloseHandle( hMapFileData );
		return 2;
	}

	// Calculate the size of the data.
	size_t textureDataSize = DDS_CalculateImageDataSize( textureHeader );

	/* READ THE DATA */
	textureData = (BYTE*)MapViewOfFile( hMapFileData,
												FILE_MAP_ALL_ACCESS,
												0,
												0,
												40 + textureDataSize
											);

	if( textureData == NULL )
	{
		_tprintf( TEXT("Couldn't fetch the texture! (%d).\n"),
				GetLastError() );
		getc(stdin);
		CloseHandle( textureData );

		UnmapViewOfFile( textureHeader );
		CloseHandle( hMapFileData );
		CloseHandle( hMapFileHeader );
		return 3;
	}

	// A hack to get straight to the texture data.
	textureData += 128; // HOX, if weird data, change to 40

	// Move the texture to a temporary vector
	std::vector<PIXDIFF> *tmp = new std::vector<PIXDIFF>();

	unsigned short y = 0;
	unsigned short x = 0;
	unsigned int rowOffset = 0;
	while( y < 1200 )
	{
		rowOffset = y*1200*4;

		for( x=0; x < 1200; ++x )
		{
			PIXDIFF p = {
				x, y,
				{
						*(textureData+rowOffset+x*4),
				        *(textureData+rowOffset+x*4+1),
						*(textureData+rowOffset+x*4+2) 
				} };

			tmp->push_back( p );
		}
		++y;
	};


	// Find differences and push them to diff.
	std::vector<PIXDIFF>::iterator it;
	std::vector<PIXDIFF>::iterator cit;
	
	if( cache->size() == tmp->size() )
	{
		for( it = tmp->begin(), cit = cache->begin(); it != tmp->end(); ++it, ++cit )
		{
			if( (*it).x >= 750 && (*it).y >= 290 )
			if( (*it).color.r != (*cit).color.r ||
				(*it).color.g != (*cit).color.g ||
				(*it).color.b != (*cit).color.b )
			{
				diff.push_back( *it );
			}
		}
	}
	else
	{
		for( it = tmp->begin(); it != tmp->end(); ++it )
		{			
			diff.push_back( *it );
		}
	}


	// Replace the old texture with the new one.
	cache->clear();
	delete cache;
	cache = tmp;


	// Return the pointer to what it was originally.
	textureData -= 128;

	// Unload and close stuff
	UnmapViewOfFile( textureData );
	UnmapViewOfFile( textureHeader );
	
	CloseHandle( hMapFileData );
	CloseHandle( hMapFileHeader );

	return diff.size();
}