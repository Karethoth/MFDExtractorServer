#include "Client.h"
#include <jpeglib.h>
#include <fstream>

Client::Client( SOCKET sockfd, ImageCache *imageCache )
{
	this->sockfd = sockfd;
	this->imageCache = imageCache;

	mode = REQUEST;
	wasLastJPEG = false;

	area.x = 0;
	area.y = 0;
	area.w = 0;
	area.h = 0;

	AREA tmpArea;
	tmpArea.x = 750;
	tmpArea.y = 290;
	tmpArea.w = 450;
	tmpArea.h = 910;
	Resize( tmpArea );

	jpegBuffer.resize( 100000 );
}



Client::~Client( void )
{
	if( sockfd != INVALID_SOCKET )
		closesocket( sockfd );

	diff.clear();
}



void Client::Resize( AREA &a )
{
	if( a.x == area.x &&
		a.y == area.y &&
		a.w == area.w &&
		a.h == area.h )
	{
		return;
	}

	mode = FORCE_TEXTURE;

	area.x = a.x;
	area.y = a.y;
	area.w = a.w;
	area.h = a.h;

	diff.resize( a.w * a.h + 1 );
	rgbBuffer.resize( a.w * a.h );
}



bool Client::Update( fd_set &readSet, fd_set &writeSet, fd_set &exceptionSet )
{
	if( FD_ISSET( sockfd, &readSet ) )
	{
		if( !HandleRead() )
			return false;
	}

	if( FD_ISSET( sockfd, &writeSet ) )
	{
		if( !HandleWrite() )
			return false;
	}

	if( FD_ISSET( sockfd, &exceptionSet ) )
	{
		printf( "Client ran in to some problems. Closing its connection.\n" );
		closesocket( sockfd );
		sockfd = INVALID_SOCKET;
		return false;
	}

	return true;
}



bool Client::HandleRead()
{
	int n = recv( sockfd, buffer, 255, 0 );
	if( n > 0 )
	{
		printf( "Bytes received: %d\n", n );
		if( n < 10 )
			return true;

		AREA tmpArea;
		tmpArea.x = (buffer[1] << 8) | (buffer[2] & 0xff);
		tmpArea.y = (buffer[3] << 8) | (buffer[4] & 0xff);
		tmpArea.w = (buffer[5] << 8) | (buffer[6] & 0xff);
		tmpArea.h = (buffer[7] << 8) | (buffer[8] & 0xff);
		Resize( tmpArea );
	}
	else if( n == 0 )
	{
		printf("Client closed connection.\n");
		closesocket( sockfd );
		sockfd = INVALID_SOCKET;
		return false;
	}
	else
	{
        if( WSAGetLastError() != WSAEWOULDBLOCK )
		{
			printf( "recv failed with error: %d\n", WSAGetLastError() );
			closesocket( sockfd );
			sockfd = INVALID_SOCKET;
			return false;
		}
	}
}



bool Client::HandleWrite()
{
	if( writeBuffer.size() > 0 )
	{
		int n = send( sockfd, &writeBuffer[0], writeBuffer.size(), 0 );
		writeBuffer.erase( writeBuffer.begin(), writeBuffer.begin()+n );
		if( writeBuffer.size() > 0 )
			return true;
		return true;
	}

	bool retval = true;
	
	if( wasLastJPEG && threshold-- > 0 )
	{
		retval = SendJPEG();
		return retval;
	}

	int diffCount = imageCache->GetDiff( area, diff );

	
	if( mode == FORCE_TEXTURE  )
	{
		imageCache->GetImage( area, diff );
		retval = SendTexture();
		mode = CONTINUOUS;
		wasLastJPEG = false;
	}

	else if( mode == CONTINUOUS && diffCount*7 < area.w * area.h * 3 )
	{
		if( wasLastJPEG )
		{
			imageCache->GetImage( area, diff );
			retval = SendTexture();
			wasLastJPEG = false;
		}
		else
		{
			retval = SendDiffVector( diffCount );
		}
	}

	else if( mode == CONTINUOUS )
	{
		//imageCache->GetImage( area, diff );
		//retval = SendTexture();
		retval = SendJPEG();
		wasLastJPEG = true;
		threshold = 25;
	}

	else if( mode == REQUEST )
	{
		imageCache->GetImage( area, diff );
		retval = SendTexture();
		mode   = CONTINUOUS;
		wasLastJPEG = false;
	}
	
	return retval;
}



bool Client::SendJPEG()
{
	imageCache->GetRGBData( area, rgbBuffer );

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;
 
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	unsigned char *buffPointer = (unsigned char*)&jpegBuffer[0];
	unsigned long buffSize = 100000;
	jpeg_mem_dest( &cinfo, &buffPointer, &buffSize );
 
	cinfo.image_width      = area.w;
	cinfo.image_height     = area.h;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults( &cinfo );
	jpeg_set_quality ( &cinfo, 25, true );
	jpeg_start_compress( &cinfo, true );
	JSAMPROW row_pointer;
 
	int row_stride = area.w;
	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer = (JSAMPROW) &rgbBuffer[cinfo.next_scanline * row_stride].r;
		jpeg_write_scanlines( &cinfo, &row_pointer, 1 );
	}

	jpeg_finish_compress( &cinfo );
	size_t jpegDataSize = 100000 - cinfo.dest->free_in_buffer;
	jpeg_destroy_compress( &cinfo );

	writeBuffer.push_back( 'J' );
	writeBuffer.push_back( jpegDataSize>>24 );
	writeBuffer.push_back( jpegDataSize>>16 );
	writeBuffer.push_back( jpegDataSize>>8 );
	writeBuffer.push_back( jpegDataSize );
	writeBuffer.push_back( ':' );

	writeBuffer.insert( writeBuffer.end(), &jpegBuffer[0], &jpegBuffer[0]+jpegDataSize );

	return true;
}



bool Client::SendTexture()
{
	int size = area.w * area.h;

	writeBuffer.push_back( 'I' );
	writeBuffer.push_back( size>>24 );
	writeBuffer.push_back( size>>16 );
	writeBuffer.push_back( size>>8 );
	writeBuffer.push_back( size );
	writeBuffer.push_back( ':' );

	std::vector<PIXDIFF>::iterator it;

	for( it = diff.begin(); it != diff.end()-1; ++it )
	{
		writeBuffer.push_back( (*it).color.r );
		writeBuffer.push_back( (*it).color.g );
		writeBuffer.push_back( (*it).color.b );
	}

	return true;
}



bool Client::SendDiffVector( int diffCount )
{
	int size = diffCount;

	writeBuffer.push_back( 'D' );
	writeBuffer.push_back( size>>24 );
	writeBuffer.push_back( size>>16 );
	writeBuffer.push_back( size>>8 );
	writeBuffer.push_back( size );
	writeBuffer.push_back( ':' );

	std::vector<PIXDIFF>::iterator it;
	for( it = diff.begin(); it != diff.end(); ++it )
	{
		// Check for the magic value
		if( (*it).x == 0xffff )
		{
			break;
		}

		unsigned short tmpX = (*it).x - area.x;
		unsigned short tmpY = (*it).y - area.y;

		writeBuffer.push_back( tmpX>>8 );
		writeBuffer.push_back( tmpX );
		writeBuffer.push_back( tmpY>>8 );
		writeBuffer.push_back( tmpY );
		writeBuffer.push_back( (*it).color.r );
		writeBuffer.push_back( (*it).color.g );
		writeBuffer.push_back( (*it).color.b );
	}
	return true;
}
