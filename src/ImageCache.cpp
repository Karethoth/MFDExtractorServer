#include "stdafx.h"
#include "ImageCache.h"


TCHAR sharedMemoryAreaName[] = TEXT( "FalconTexturesSharedMemoryArea" );


ImageCache::ImageCache()
{
	// Set up and allocate the vectors
	cache      = new std::vector<PIXDIFF>(1200*1200);
	oldCache   = new std::vector<PIXDIFF>(1200*1200);
	diff       = new std::vector<PIXDIFF>(1200*1200+1); // +1 for the magic value~

	textureHeader = NULL;
	textureData   = NULL;
}



ImageCache::~ImageCache()
{
	cache->clear();
	delete cache;

	oldCache->clear();
	delete oldCache;

	diff->clear();
	delete diff;
}



int ImageCache::Update()
{
	// Check if we haven't opened connection to
	// the shared memory yet.
	if( textureHeader == NULL ||
		textureData   == NULL )
	{
		if( !OpenMemory() )
			return -1;
	}
	// Swap the oldCache and cache.
	// The oldCache will be overwritten by new data.
	std::vector<PIXDIFF> *swapCache = oldCache;
	oldCache = cache;
	cache = swapCache;
	swapCache = NULL;
	
	// A hack to get straight to the texture data.
	textureData += 128; // HOX, if weird data, change to 40

	// Write the texture over the texture in the cache vector
	std::vector<PIXDIFF>::iterator tmp = cache->begin();

	unsigned short y = 0;
	unsigned short x = 0;
	unsigned int rowOffset = 0;
	while( y < 1200 )
	{
		rowOffset = y*1200*4;

		for( x=0; x < 1200; ++x )
		{
			(*tmp).x = x;
			(*tmp).y = y;
			(*tmp).color.r = *(textureData+rowOffset+x*4);
			(*tmp).color.g = *(textureData+rowOffset+x*4+1);
			(*tmp).color.b = *(textureData+rowOffset+x*4+2);
			++tmp;
		}
		++y;
	};

	// Find differences and use them to overwrite old differences in the diff.
	std::vector<PIXDIFF>::iterator it;
	std::vector<PIXDIFF>::iterator cit;
	std::vector<PIXDIFF>::iterator dit;

	int diffCounter=0;
	
	for( it = oldCache->begin(), cit = cache->begin(), dit = diff->begin();
		 it != oldCache->end();
		 ++it, ++cit  )
	{
		if( (*it).color.r != (*cit).color.r ||
			(*it).color.g != (*cit).color.g ||
			(*it).color.b != (*cit).color.b )
		{
			(*dit).x = (*cit).x;
			(*dit).y = (*cit).y;
			(*dit).color.r = (*cit).color.r;
			(*dit).color.g = (*cit).color.g;
			(*dit).color.b = (*cit).color.b;
			++diffCounter;
			++dit;
		}
	}

	// Insert the magic value:
	(*dit).x = 0xffff;

	// Return the pointer to what it was originally.
	textureData -= 128;

	// Unload and close stuff
	//UnmapViewOfFile( textureData );
	//UnmapViewOfFile( textureHeader );
	
	//CloseHandle( hMapFileData );
	//CloseHandle( hMapFileHeader );

	// Return the count of changes we got
	return diffCounter;
}



bool ImageCache::OpenMemory()
{
	hMapFileHeader = OpenFileMapping(
					FILE_MAP_ALL_ACCESS,
					FALSE,
					sharedMemoryAreaName
				);

	if( hMapFileHeader == NULL )
	{
		_tprintf( TEXT("Could not open file mapping object (%d).\n"),
				GetLastError() );
		return false;
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
		return false;
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
		return false;
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
		return false;
	}
	return true;
}



int ImageCache::GetDiff( AREA &area, std::vector<PIXDIFF> &dest )
{
	if( area.w == 0 ||
		area.h == 0 )
	{
		dest.begin()->x = 0xffff;
		return 0;
	}

	int diffCount = 0;

	std::vector<PIXDIFF>::iterator it;
	std::vector<PIXDIFF>::iterator dit;

	for( it = diff->begin(), dit = dest.begin(); it != diff->end(); ++it )
	{
		if( (*it).x == 0xffff )
		{
			break;
		}
		if( (*it).x >= area.x && (*it).x < (area.x + area.w) &&
			(*it).y >= area.y && (*it).y < (area.y + area.h) )
		{
			(*dit).x = (*it).x;
			(*dit).y = (*it).y;
			(*dit).color.r = (*it).color.r;
			(*dit).color.g = (*it).color.g;
			(*dit).color.b = (*it).color.b;
			++dit;
			++diffCount;
		}
	}
	(*dit).x = 0xffff;

	return diffCount;
}



int ImageCache::GetImage( AREA &area, std::vector<PIXDIFF> &dest  )
{
	std::vector<PIXDIFF>::iterator it;
	std::vector<PIXDIFF>::iterator dit;

	int counter=0;

	for( it = cache->begin(), dit = dest.begin(); it != cache->end(); ++it )
	{
		if( (*it).x >= area.x && (*it).x < (area.x + area.w) &&
			(*it).y >= area.y && (*it).y < (area.y + area.h) )
		{
			(*dit).x = (*it).x;
			(*dit).y = (*it).y;
			(*dit).color.r = (*it).color.r;
			(*dit).color.g = (*it).color.g;
			(*dit).color.b = (*it).color.b;
			++dit;
			++counter;
		}
	}

	return counter;
}



int ImageCache::GetRGBData( AREA &area, std::vector<PIXEL> &dest  )
{
	std::vector<PIXDIFF>::iterator it;
	std::vector<PIXEL>::iterator dit;

	int counter=0;

	for( it = cache->begin(), dit = dest.begin(); it != cache->end(); ++it )
	{
		if( (*it).x >= area.x && (*it).x < (area.x + area.w) &&
			(*it).y >= area.y && (*it).y < (area.y + area.h) )
		{
			(*dit).r = (*it).color.b;
			(*dit).g = (*it).color.g;
			(*dit).b = (*it).color.r;
			++dit;
			++counter;
		}
	}

	return counter;
}
