// SVC - Simple Voice Communicator 
// funkcja main, uruchamia kontrolery klienta SVC
// autor: Marcin Frankowski


#include "ClientApp.hpp"
#include "AudioController.hpp"
#include "inputCheck.hpp"


int main(int argc, char** argv)
{
	if(checkPort(argc, argv[1]) != 0)
	{
		std::cout<<"Niepoprawny port!"<<std::endl;
		exit(0);
	}	
    ClientApp* clientApp = new ClientApp(atoi(argv[1]));
    std::system("clear");
    std::cout << "Wpisz polecenie. Aby wyświetlić dostępne komendy wpisz 'help'" << std::endl;
    clientApp->commandThread();
    clientApp->initializeEnvironment();
}

