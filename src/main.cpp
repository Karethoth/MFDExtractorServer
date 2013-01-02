// F4SharedTextureReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

#include "ImageCache.h"
#include "Server.h"

#include <windows.h>


int _tmain( int argc, _TCHAR* argv[] )
{
	ImageCache ic;
	Server serv;
	
	if( !serv.Start() )
	{
		printf( "Starting the server failed!\n" );
		return 1;
	}

	while( 1 )
	{
		if( serv.Update() )
		{
			int diffCount = ic.Update();

			// Calculate which is the most efficient way to send the data.
			if( diffCount*7 >= ic.GetImage()->size()*3 || serv.buffer[0] == 'I' )
			{
				// Send the whole image
				serv.SendTexture( ic.GetImage() );
			}
			else
			{
				// Send only the updated parts
				while( ic.Update() >= 0 && serv.SendDiffVector( ic.GetDiff() ) )
				{
					Sleep( 50 );
				}
			}
		}

		Sleep( 100 );
	}

	getc( stdin );
	return 0;
}
