#include "stdafx.h"
#include "INI.h"

#include <fstream>
#include <algorithm>


bool INI::Load( std::string filepath )
{
	std::ifstream inp( filepath, std::ios::in );
	if( !inp.is_open() )
	{
		printf( "Couldn't open file '%s'.\n", filepath.c_str() );
		return false;
	}

	std::string line;
	std::string key, value;
	size_t eq;

	line.resize( 200 );

	while( inp.getline( &line[0], 200 ) )
	{
		if( (eq = line.find( "=" )) > 0 )
		{
			if( eq >= line.length()-1 )
				continue;

			key   = line.substr( 0, eq-1 );
			value = line.substr( eq+1 );

			key.erase( std::remove_if( key.begin(), key.end(), isspace ), key.end() );
			value.erase( std::remove_if( value.begin(), value.end(), isspace ), value.end() );

			if( key.length() <= 0 ||
				value.length() <= 0 )
				continue;

			pairs[key] = value;
		}
	}

	inp.close();

	return true;
}



std::string& INI::operator[]( const std::string &key )
{
	return pairs[key];
}
