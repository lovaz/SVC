#ifndef AUDIOOUTCONTROLLER_HPP
#define AUDIOOUTCONTROLLER_HPP

#include "portaudio.h"
#include "BlockingQueue.hpp"
#include "Sample.hpp"
#include <string.h> 
#include <fstream>



class AudioOutController
{
private:
    
    float sampleBlock[FRAMES_PER_BUFFER];
    bool threadRunning;
    PaStream* stream;
    PaError error;
    PaStreamParameters outputParameters;
    std::fstream audioLog;

public:

    AudioOutController(); 
    ~AudioOutController();
    void clearBuffer(float*, int);
    void initializeAudio();
    void closeAudio();
    void playAudio(BlockingQueue<Sample>& blockingQueue);
    void openStream();
    void catchPaError();
    void catchPaErrorPlay();
    void stopThread();
    void startThread();
};

#endif

