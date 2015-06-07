// SVC - Simple Voice Communicator 
// funkcje sprawdzające dane wprowadzane do programu
// autor: Marcin Frankowski


#ifndef INPUTCHECK_HPP
#define INPUTCHECK_HPP

#include <iostream>
#include <ios>
#include <sstream>

int checkPort(int argc, char* port)
{
	if(argc < 2 || argc > 3)
	{
		return -1;
	}
	else
	{
		std::istringstream in(port);
		unsigned short i;
		if(in >> i && in.eof())
		{
			return 0;
		}
		else
		{
			return -1;
		}		
	}
	return -1;
}


#endif