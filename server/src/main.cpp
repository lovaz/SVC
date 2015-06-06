// SVC - Simple Voice Communicator 
// funkcja main, interfejs do obsługi serwera
// autor: Marcin Frankowski


#include "ServerController.hpp"
#include "inputCheck.hpp"
#include <unistd.h>
#include <thread>
#include <iostream>


int main(int argc, char** argv)
{
	if(checkPort(argc, argv[1]) != 0)
	{
		std::cout<<"Niepoprawny port!"<<std::endl;
		exit(0);
	}
    std::cout << "SVC NAT SERVER"<< std::endl;
    ServerController* serverController = new ServerController();
    serverController->initTcpServer(atoi(argv[1]));
    std::thread actcThread(&ServerController::acceptTcpConnections, serverController, atoi(argv[1]));
    std::string input;
    while(1)
	{
		std::getline(std::cin, input);
		if(input == "users")
		{
			serverController->printUsers();
		}
		if(input == "exit")
		{
			serverController->cleanSockets();
			exit(0);
		}
	}
	actcThread.join();
}
