#include "NetworkController.hpp"
#include <unistd.h>
#include <thread>
#include "ThreadSafeMap.hpp"
#include <iostream>


int main(int argc, char** argv)
{
    std::cout << "SVC NAT SERVER"<< std::endl;
    NetworkController* networkController = new NetworkController();
    networkController->initTcpServer(atoi(argv[1]));
    std::thread actcThread(&NetworkController::acceptTcpConnections, networkController, atoi(argv[1]));
    std::string input;
    while(1)
	{
		std::getline(std::cin, input);
		if(input == "users")
		{
			networkController->printUsers();
		}
		if(input == "exit")
		{
			exit(0);
		}
	}
	actcThread.join();
}
