#include "Client.h"


Client::Client( SOCKET sockfd, ImageCache *imageCache )
{
	this->sockfd = sockfd;
	this->imageCache = imageCache;

	mode = REQUEST;

	area.x = 0;
	area.y = 0;
	area.w = 1200;
	area.h = 1200;
}



Client::~Client( void )
{
	if( sockfd != INVALID_SOCKET )
		closesocket( sockfd );
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

		area.x = (buffer[1] << 8) | (buffer[2] & 0xff);
		area.y = (buffer[3] << 8) | (buffer[4] & 0xff);
		area.w = (buffer[5] << 8) | (buffer[6] & 0xff);
		area.h = (buffer[7] << 8) | (buffer[8] & 0xff);
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
	std::vector<PIXDIFF*> *diff = imageCache->GetDiff( area );
	
	if( mode == CONTINUOUS && diff->size()*7 < area.w * area.h * 3 )
	{
		retval = SendDiffVector( diff );
	}

	else if( mode == CONTINUOUS )
	{
		retval = SendTexture( imageCache->GetImage( area ) );
	}

	else if( mode == REQUEST )
	{
		retval = SendTexture( imageCache->GetImage( area ) );
		mode   = CONTINUOUS;
	}
	
	diff->clear();
	delete diff;

	return retval;
}



bool Client::SendTexture( std::vector<PIXDIFF*> *texture )
{
	int size = texture->size();

	std::vector<char> message;
	message.push_back( 'I' );
	message.push_back( size>>24 );
	message.push_back( size>>16 );
	message.push_back( size>>8 );
	message.push_back( size );
	message.push_back( ':' );

	std::vector<PIXDIFF*>::iterator it;

	for( it = texture->begin(); it != texture->end(); ++it )
	{
		message.push_back( (*it)->color.r );
		message.push_back( (*it)->color.g );
		message.push_back( (*it)->color.b );
	}

    int n = send( sockfd, &message[0], message.size(), 0 );
    if( n == SOCKET_ERROR)
	{
        if( WSAGetLastError() != WSAEWOULDBLOCK )
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket( sockfd );
			sockfd = INVALID_SOCKET;
			return false;
		}
    }

	return true;
}



bool Client::SendDiffVector( std::vector<PIXDIFF*> *diff )
{
	int size = diff->size();

	std::vector<char> message;
	message.push_back( 'D' );
	message.push_back( size>>24 );
	message.push_back( size>>16 );
	message.push_back( size>>8 );
	message.push_back( size );
	message.push_back( ':' );

	std::vector<PIXDIFF*>::iterator it;
	for( it = diff->begin(); it != diff->end(); ++it )
	{
		unsigned short tmpX = (*it)->x - area.x;
		unsigned short tmpY = (*it)->y - area.y;

		message.push_back( tmpX>>8 );
		message.push_back( tmpX );
		message.push_back( tmpY>>8 );
		message.push_back( tmpY );
		message.push_back( (*it)->color.r );
		message.push_back( (*it)->color.g );
		message.push_back( (*it)->color.b );
	}

    int n = send( sockfd, &message[0], message.size(), 0 );
    if( n == SOCKET_ERROR)
	{
        if( WSAGetLastError() != WSAEWOULDBLOCK )
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket( sockfd );
			sockfd = INVALID_SOCKET;
			return false;
   		}
	}
	return true;
}
