#include "stdafx.h"
#include "DDS.h"


extern size_t DDS_CalculateImageDataSize( DDS_HEADER *header )
{
	// no need to calculate for the pitch nor stride.
	// Falcon is nice enough to use plain 32bit format without any stride.
	return header->dwWidth * header->dwHeight * (header->ddspf.dwRGBBitCount/8);
}
