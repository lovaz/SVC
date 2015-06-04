#include "ClientApp.hpp"
#include "AudioController.hpp"
#include <unistd.h>
#include <thread>
#include <iostream>
#include <list>
#include <pthread.h>


int main(int argc, char** argv)
{
    ClientApp* clientApp = new ClientApp(atoi(argv[1]));
    std::system("clear");
    std::cout << "Wpisz polecenie. Aby wyświetlić dostępne komendy wpisz 'help'" << std::endl;
    clientApp->commandThread();
    clientApp->initializeEnvironment();
}

