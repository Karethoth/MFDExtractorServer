#pragma once
#include "ImageCache.h"

#include <WinSock2.h>
#include <vector>


enum CLIENT_MODE
{
	REQUEST,
	CONTINUOUS
};



class Client
{
private:
	SOCKET      sockfd;
	AREA        area;
	CLIENT_MODE mode;
	ImageCache *imageCache;

	char buffer[255];
	std::vector<char> writeBuffer;
	
	//std::vector<char> buffer;


protected:
	bool HandleRead();
	bool HandleWrite();

	bool SendTexture( std::vector<PIXDIFF*> *texture );
	bool SendDiffVector( std::vector<PIXDIFF*> *diff );


public:
	Client( SOCKET sockfd, ImageCache *imageCache );
	~Client( void );

	bool Update( fd_set &readSet, fd_set &writeSet, fd_set &exceptionSet );

	SOCKET GetSocket()
	{ return sockfd; }
};
