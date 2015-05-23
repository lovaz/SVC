#ifndef AUDIOCONTROLLER_HPP
#define AUDIOCONTROLLER_HPP

#include "portaudio.h"
#include "BlockingQueue.hpp"
#include <string.h> 

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_CHANNELS    (2)
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)


class Sample
{
private:
    char sample[FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE];
public:
    void setSample(char* data)
    {
        memcpy(sample, data, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE);
    }
    char* getSample()
    {
        return sample;
    }
};

class AudioController
{
private:
    
    char* sampleBlock;
    int bufferSize=FRAMES_PER_BUFFER;
    PaStream* stream;
    PaError error;
    PaStreamParameters inputParameters, outputParameters;
    char* record;
    // BlockingQueue<Sample> blockingQueue;


    //std::thread;
    //blockingqueue nadan z gory przez AppClient
public:

    AudioController(); //audio controller bedzie wstawał od blocking queue
    ~AudioController();
    void clearBuffer(char*, int);
    void initializeAudio();
    void IOAudioFunction();
    void closeAudio();
    void recordAudio(BlockingQueue<Sample>& blockingQueue);
    void playAudio(BlockingQueue<Sample>& blockingQueue);
    void openStream();

    void catchPa_Error();
};

#endif
