#include "ClientApp.hpp"
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


int main(int argc, char** argv)
{
    ClientApp* clientApp = new ClientApp(atoi(argv[1]));
    std::system("clear");
    std::thread t1(&ClientApp::commandHandler, clientApp);
    clientApp->initializeEnvironment();
    t1.join();
}

