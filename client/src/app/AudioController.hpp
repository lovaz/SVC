#ifndef AUDIOCONTROLLER_HPP
#define AUDIOCONTROLLER_HPP

#include "portaudio.h"
#include "BlockingQueue.hpp"
#include "Sample.hpp"
#include <string.h> 



class AudioController
{
private:
    
    //char* sampleBlock;
    float floatBlock[FRAMES_PER_BUFFER];
    int bufferSize=FRAMES_PER_BUFFER;
    bool threadRunning;
    PaStream* stream;
    PaError error;
    PaStreamParameters inputParameters, outputParameters;
   // char* record;

public:

    AudioController(); 
    ~AudioController();
    void clearBuffer(float*, int);
    void initializeAudio();
    void IOAudioFunction();
    void closeAudio();
    void recordAudio(BlockingQueue<Sample>& blockingQueue);
    void playAudio(BlockingQueue<Sample>& blockingQueue);
    void openStream();
    void catchPa_Error();
    void stopThread();
    void startThread();
};

#endif

