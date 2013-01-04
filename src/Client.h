#pragma once
#include "ImageCache.h"

#include <WinSock2.h>
#include <vector>


enum CLIENT_MODE
{
	REQUEST,
	CONTINUOUS,
	FORCE_TEXTURE
};



class Client
{
private:
	SOCKET      sockfd;
	AREA        area;
	CLIENT_MODE mode;
	ImageCache *imageCache;
	std::vector<PIXDIFF> diff;

	char buffer[255];
	std::vector<char> writeBuffer;
	
	//std::vector<char> buffer;


protected:
	bool HandleRead();
	bool HandleWrite();

	bool SendTexture();
	bool SendDiffVector( int diffCount );

	void Resize( AREA &a );

public:
	Client( SOCKET sockfd, ImageCache *imageCache );
	~Client( void );

	bool Update( fd_set &readSet, fd_set &writeSet, fd_set &exceptionSet );

	SOCKET GetSocket()
	{ return sockfd; }
};
