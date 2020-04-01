#pragma once

#include <map>
#include <string>


class INI
{
private:
	std::map<std::string, std::string> pairs;

public:
	bool Load( std::string filepath );

	std::string& operator[] ( const std::string& key );
};
