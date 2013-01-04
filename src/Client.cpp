#include "Client.h"


Client::Client( SOCKET sockfd, ImageCache *imageCache )
{
	this->sockfd = sockfd;
	this->imageCache = imageCache;

	mode = REQUEST;

	area.x = 0;
	area.y = 0;
	area.w = 0;
	area.h = 0;

	AREA tmpArea;
	tmpArea.x = 0;
	tmpArea.y = 0;
	tmpArea.w = 1200;
	tmpArea.h = 1200;
	Resize( tmpArea );
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

	diff.clear();
	diff.resize( a.w * a.h + 1 );
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
	}

	bool retval = true;

	int diffCount = imageCache->GetDiff( area, diff );
	
	if( mode == FORCE_TEXTURE )
	{
		imageCache->GetImage( area, diff );
		retval = SendTexture();
		mode = CONTINUOUS;
	}

	else if( mode == CONTINUOUS && diffCount*7 < area.w * area.h * 3 )
	{
		retval = SendDiffVector( diffCount );
	}

	else if( mode == CONTINUOUS )
	{
		imageCache->GetImage( area, diff );
		retval = SendTexture();
	}

	else if( mode == REQUEST )
	{
		imageCache->GetImage( area, diff );
		retval = SendTexture();
		mode   = CONTINUOUS;
	}
	
	return retval;
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
