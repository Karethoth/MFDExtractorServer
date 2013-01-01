#pragma once
#include "stdafx.h"

#include "BMP.h"
#include <fstream>

using namespace std;



bool SaveDDSFromMemoryToBMP(	std::string filepath, DDS_HEADER *header, BYTE *data )
{
	ofstream out( filepath, ofstream::binary );
	if( !out.is_open() )
	{
		return false;
	}
	
	size_t padding = header->dwWidth%4;
	if( padding != 0 )
		padding = 4-padding;


	BMP_HEADER bmpHeader =
	{
		0x4d42,
		0,
		0,
		0,
		0x36,
		header->dwWidth*header->dwHeight + header->dwHeight*padding,
		header->dwWidth,
		header->dwHeight,
		1,
		24,
		0,0,0,0,0,0
	};

	out.write( (const char*)&bmpHeader, 0x36 );

	size_t y = header->dwHeight;
	size_t x = 0;
	size_t rowOffset = 0;

	do
	{
		--y;
		rowOffset = y * (header->dwWidth * 4);

		for( x=0; x < header->dwWidth; ++x )
		{
			out.write( (const char*)(data+rowOffset+x*4), 3 );
		}

		for( size_t padder=0; padder < padding; ++padder )
			out.put(0x255);

	} while( y != 0 );

	out.flush();
	out.close();
	return true;
}
