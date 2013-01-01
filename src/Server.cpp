#include "stdafx.h"
#include "Server.h"



Server::Server(void)
{
	listenSocket = INVALID_SOCKET;
	clientSocket = INVALID_SOCKET;
}



Server::~Server(void)
{
}



bool Server::Start()
{
	int iResult;

	struct addrinfo *result = NULL;
    struct addrinfo hints;

    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo( "192.168.2.2", "45001", &hints, &result );
    if ( iResult != 0 )
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
    if( listenSocket == INVALID_SOCKET )
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo( result );
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if( iResult == SOCKET_ERROR )
	{
        printf( "bind failed with error: %d\n", WSAGetLastError() );
        freeaddrinfo( result );
        closesocket( listenSocket );
        WSACleanup();
        return 1;
    }

    freeaddrinfo( result );

    iResult = listen( listenSocket, 1 );
    if( iResult == SOCKET_ERROR )
	{
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket( listenSocket );
        WSACleanup();
        return 1;
    }
	return true;
}



bool Server::Update()
{
	while( clientSocket == INVALID_SOCKET )
	{
		printf( "Waiting for connection..\n" );
		// Accept a client socket
		clientSocket = accept( listenSocket, NULL, NULL );
		if( clientSocket == INVALID_SOCKET )
		{
			printf( "accept failed with error: %d\n", WSAGetLastError() );
			return false;
		}
		printf( "Client connected!\n" );
	}

	int n = recv( clientSocket, buffer, 255, 0 );
    if( n > 0 )
	{
        printf( "Bytes received: %d\n", n );
    }
    else if( n == 0 )
	{
        printf("Client closed connection.\n");
		closesocket( clientSocket );
		clientSocket = INVALID_SOCKET;
		return false;
	}
	else
	{
        printf( "recv failed with error: %d\n", WSAGetLastError() );
        closesocket( clientSocket );
		clientSocket = INVALID_SOCKET;
        return false;
    }
	return true;
}



bool Server::SendTexture( std::vector<PIXDIFF> *texture )
{
	int size = texture->size();

	std::vector<char> message;
	message.push_back( 'I' );
	message.push_back( size>>24 );
	message.push_back( size>>16 );
	message.push_back( size>>8 );
	message.push_back( size );
	message.push_back( ':' );

	std::vector<PIXDIFF>::iterator it;
	for( it = texture->begin(); it != texture->end(); ++it )
	{
		message.push_back( (*it).color.r );
		message.push_back( (*it).color.g );
		message.push_back( (*it).color.b );
	}

    int n = send( clientSocket, &message[0], message.size(), 0 );
    if( n == SOCKET_ERROR)
	{
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket( clientSocket );
		clientSocket = INVALID_SOCKET;
        return false;
    }

	return true;
}



bool Server::SendDiffVector( std::vector<PIXDIFF> *diff )
{
	int size = diff->size();

	std::vector<char> message;
	message.push_back( 'D' );
	message.push_back( size>>24 );
	message.push_back( size>>16 );
	message.push_back( size>>8 );
	message.push_back( size );
	message.push_back( ':' );

	std::vector<PIXDIFF>::iterator it;
	for( it = diff->begin(); it != diff->end(); ++it )
	{
		message.push_back( (*it).x>>8 );
		message.push_back( (*it).x );
		message.push_back( (*it).y>>8 );
		message.push_back( (*it).y );
		message.push_back( (*it).color.r );
		message.push_back( (*it).color.g );
		message.push_back( (*it).color.b );
	}
    int n = send( clientSocket, &message[0], message.size(), 0 );
    if( n == SOCKET_ERROR)
	{
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket( clientSocket );
		clientSocket = INVALID_SOCKET;
        return false;
    }

	return true;
}
