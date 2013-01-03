#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "ImageCache.h"
#include "Client.h"


class Server
{
  private:
	std::vector<Client*> clientList;
	WSADATA wsaData;

    SOCKET listenSocket;

	FD_SET writeSet;
	FD_SET readSet;
	FD_SET exceptionSet;

	ImageCache *imageCache;


  protected:
	bool SetupFDSets();


  public:
	Server( ImageCache *imageCache );
	~Server( void );

	bool Start( const char *port );
	bool Update();
};
