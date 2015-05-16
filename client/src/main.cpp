#include "NetworkController.hpp"
#include <thread>

int main(int argc, char** argv)
{
    BlockingQueue<char*> blockingQueue;
    Connection* NeCo = new Connection();

    if(argc == 2)
    {
        NeCo->tcpServer(atoi(argv[1]));
        NeCo->tcpRecv();
    }
    else
    {
        NeCo->tcpClient(atoi(argv[2]),argv[1],sizeof(argv[1])/sizeof(char));
        NeCo->tcpSend();
    }

    return 0;
}
