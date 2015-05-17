#include "NetworkController.hpp"
#include <thread>

int main(int argc, char** argv)
{
    BlockingQueue<char*> blockingQueue;
    Connection* NeCo = new Connection();

    if(argc == 2)
    {
        NeCo->tcpServer(atoi(argv[1]));
        std::thread t1(&Connection::tcpRecv, NeCo);
        std::thread t2(&Connection::udpRecv, NeCo);
        std::thread t3(&Connection::udpSend, NeCo, argv[1]);
        t1.join();
        t2.join();
        t3.join();
    }
    else
    {
        NeCo->tcpClient(atoi(argv[2]),argv[1],sizeof(argv[1])/sizeof(char));
        std::thread t1(&Connection::tcpSend, NeCo);
        std::thread t2(&Connection::udpSend, NeCo, argv[1]);
        std::thread t3(&Connection::udpRecv, NeCo);
        t1.join();
        t2.join();
        t3.join();
    }
    return 0;
}
