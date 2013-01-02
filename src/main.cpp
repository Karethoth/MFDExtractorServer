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
#include "INI.h"

#include <windows.h>


bool CheckConfig( INI &ini )
{
	if( ini["PORT"].size() <= 0 )
	{
		printf( "Missing PORT from the configuration file, please add it!\n" );
		return false;
	}
	if( ini["SLEEP"].size() <= 0 )
	{
		printf( "Missing SLEEP from the configuration file, please add it!\n" );
		return false;
	}

	return true;
}



int _tmain( int argc, _TCHAR* argv[] )
{
	/* Load the configuration */
	INI ini;
	if( !ini.Load( "config.ini" ) )
	{
		printf( "Failed to load configuration, exiting..\nPress enter to continue.\n" );
		getc( stdin );
		return 1;
	}

	/* Check if we are missing some values */
	if( !CheckConfig( ini ) )
	{
		printf( "Configuration missing values, exiting..\nPress enter to continue.\n" );
		getc( stdin );
		return 1;
	}

	printf( "Configuration loaded.\n" );



	/* Start the server */
	Server serv;
	
	if( !serv.Start( ini["PORT"].c_str() ) )
	{
		printf( "Starting the server failed!\n" );
		return 1;
	}



	/* Define the variable for ImageCache */
	ImageCache ic;



	/* Grab the SLEEP from the ini to a variable. */
	size_t sleepDelay = atoi( ini["SLEEP"].c_str() );



	/* Das loop */
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
					Sleep( sleepDelay );
				}
			}
		}

		Sleep( 100 );
	}

	getc( stdin );
	return 0;
}
