#include "stdafx.h"
#include "Server.h"



Server::Server( ImageCache *imageCache )
{
	this->imageCache = imageCache;
	listenSocket = INVALID_SOCKET;
}



Server::~Server( void )
{
}



bool Server::SetupFDSets()
{
	FD_ZERO( &readSet );
    FD_ZERO( &writeSet );
    FD_ZERO( &exceptionSet );

	if( listenSocket != INVALID_SOCKET )
	{
		FD_SET( listenSocket, &readSet );
        FD_SET( listenSocket, &exceptionSet );
    }
	else
	{
		return false;
	}

    // Add client connections
    std::vector<Client*>::iterator it = clientList.begin();
	for( it = clientList.begin(); it != clientList.end(); ++it )
	{
		FD_SET( (*it)->GetSocket(), &readSet );
        FD_SET( (*it)->GetSocket(), &writeSet );
		FD_SET( (*it)->GetSocket(), &exceptionSet );
	}

	return true;
}



bool Server::Start( const char *port )
{
	int iResult;

	struct addrinfo *result = NULL;
    struct addrinfo hints;

    
    // Initialize Winsock
    iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if (iResult != 0)
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return false;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo( NULL, port, &hints, &result );
    if ( iResult != 0 )
	{
        printf( "getaddrinfo failed with error: %d\n", iResult );
        WSACleanup();
        return false;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
    if( listenSocket == INVALID_SOCKET )
	{
        printf( "socket failed with error: %ld\n", WSAGetLastError() );
        freeaddrinfo( result );
        WSACleanup();
        return false;
    }

    // Setup the TCP listening socket
    iResult = bind( listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if( iResult == SOCKET_ERROR )
	{
        printf( "bind failed with error: %d\n", WSAGetLastError() );
        freeaddrinfo( result );
        closesocket( listenSocket );
        WSACleanup();
        return false;
    }

    freeaddrinfo( result );

	u_long noBlock = 1;
	if( ioctlsocket( listenSocket, FIONBIO, &noBlock ) == SOCKET_ERROR )
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return false;
	}

    iResult = listen( listenSocket, 1 );
    if( iResult == SOCKET_ERROR )
	{
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket( listenSocket );
        WSACleanup();
        return false;
    }

	printf( "Listening for connections now.\n" );

	return true;
}



bool Server::Update()
{
	SetupFDSets();
	select( 0, &readSet, &writeSet, &exceptionSet, 0 );

	/* Check for a new connection */
	if( FD_ISSET( listenSocket, &readSet ) )
	{
		SOCKET newConnection = accept( listenSocket, NULL, NULL );
		if( newConnection != INVALID_SOCKET)
		{
			printf( "Client connected.\n" );

			Client *newClient = new Client( newConnection, imageCache );
			clientList.push_back( newClient );
			
			// Mark the socket as non-blocking, for safety.
			u_long noBlock = 1;
			ioctlsocket( newConnection, FIONBIO, &noBlock );
		}
		else
		{
            if( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				printf( "accept failed with error: %d. The listening socket is pretty much dead.\n", WSAGetLastError() );
				return false;
			}
		}
	}


	/* Check the clients */
	std::vector<Client*>::iterator it;
	for( it = clientList.begin(); it != clientList.end(); )
	{
		if( (*it)->GetSocket() == INVALID_SOCKET )
		{
			delete *it;
			it = clientList.erase( it );
			continue;
		}

		(*it)->Update( readSet, writeSet, exceptionSet );

		++it;
	}

	return true;
}
