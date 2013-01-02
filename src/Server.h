#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "ImageCache.h"


class Server
{
  private:
	WSADATA wsaData;

    SOCKET listenSocket;
    SOCKET clientSocket;

  public:
	Server( void );
	~Server( void );

	bool Start( const char *port );
	bool Update();
	bool SendTexture( std::vector<PIXDIFF> *texture );
	bool SendDiffVector( std::vector<PIXDIFF> *diff );

	char buffer[255];
};
