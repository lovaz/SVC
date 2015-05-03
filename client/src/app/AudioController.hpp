#ifndef AUDIOCONTROLLER_HPP
#define AUDIOCONTROLLER_HPP

#include "portaudio.h"

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_CHANNELS    (2)
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)


class AudioController
{
private:
    
    char* sampleBlock;
    int bufferSize=FRAMES_PER_BUFFER;
    PaStream* stream;
    PaError error;
    PaStreamParameters inputParameters, outputParameters;
    char* record;

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