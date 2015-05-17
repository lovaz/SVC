#include "NetworkController.hpp"
#include <thread>

int main(int argc, char** argv)
{
    BlockingQueue<Sample> blockingQueueSend;
    BlockingQueue<Sample> blockingQueueRecv;
    BlockingQueue<Sample> blockingQueue;
    Connection* NeCo = new Connection();


    if(argc == 2)
    {
        NeCo->tcpServer(atoi(argv[1]));
        std::thread t1(&Connection::tcpRecv, NeCo);
        std::thread t2(&Connection::udpSend, NeCo, std::ref(blockingQueueSend));
        std::thread t3(&Connection::udpRecv, NeCo, std::ref(blockingQueueRecv));
        std::thread t4(&Connection::sampleFactory, NeCo, std::ref(blockingQueue));
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
    else
    {
        NeCo->tcpClient(atoi(argv[2]),argv[1],sizeof(argv[1])/sizeof(char));
        std::thread t1(&Connection::tcpSend, NeCo);
        std::thread t2(&Connection::udpSend, NeCo, std::ref(blockingQueueSend));
        std::thread t3(&Connection::udpRecv, NeCo, std::ref(blockingQueueRecv));
        std::thread t4(&Connection::sampleFactory, NeCo, std::ref(blockingQueueSend));
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }



    return 0;
}
