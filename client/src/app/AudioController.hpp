#ifndef AUDIOCONTROLLER_HPP
#define AUDIOCONTROLLER_HPP

#include "portaudio.h"

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_CHANNELS    (2)
#define PA_SAMPLE_TYPE  paUInt8
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (128)


class AudioController
{
private:
    
    char* sampleBlock;
    int bufferSize=FRAMES_PER_BUFFER;
    PaStream* stream;
    PaError error;
    PaStreamParameters inputParameters, outputParameters;

    //std::thread;
    //blockingqueue nadan z gory przez AppClient
public:

    AudioController(); //audio controller bedzie wstawał od blocking queue
    ~AudioController();
    void clearBuffer(char*, int);
    void initializeAudio();
    void IOAudioFunction();
    void closeAudio();


    void catchPa_Error();
};


#endif