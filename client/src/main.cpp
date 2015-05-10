#include "AudioController.hpp"
#include <thread>

int main(int argc, char** argv)
{
	BlockingQueue<Sample> blockingQueue;
    AudioController* AC1 = new AudioController();
    AudioController* AC2 = new AudioController();
    AC1->initializeAudio();
    AC1->openStream();
    AC2->initializeAudio();
    AC2->openStream();
    std::thread t1(&AudioController::recordAudio, AC1,std::ref(blockingQueue));
    std::thread t2(&AudioController::playAudio, AC2,std::ref(blockingQueue));
    t1.join();
    t2.join();
    AC1->closeAudio();
    AC2->closeAudio();

    return 0;
}
