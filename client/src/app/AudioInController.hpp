#ifndef AUDIOINCONTROLLER_HPP
#define AUDIOINCONTROLLER_HPP

#include "portaudio.h"
#include "BlockingQueue.hpp"
#include "Sample.hpp"
#include <string.h> 
#include <fstream>



class AudioInController
{
private:
    
    float sampleBlock[FRAMES_PER_BUFFER];
    bool threadRunning;
    PaStream* stream;
    PaError error;
    PaStreamParameters inputParameters;
    std::fstream audioLog;

public:

    AudioInController(); 
    ~AudioInController();
    void clearBuffer(float*, int);
    void initializeAudio();
    void closeAudio();
    void recordAudio(BlockingQueue<Sample>& blockingQueue);
    void openStream();
    void catchPaError();
    void catchPaErrorRec();
    void stopThread();
    void startThread();
};

#endif