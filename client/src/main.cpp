#include "AudioController.hpp"


int main(int argc, char** argv)
{
    AudioController* AC = new AudioController();
    AC->initializeAudio();
    AC->IOAudioFunction();
    AC->closeAudio();
    return 0;
}