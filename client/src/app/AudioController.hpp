#ifndef AUDIOCONTROLLER_HPP
#define AUDIOCONTROLLER_HPP

#include "portaudio.h"
#include "BlockingQueue.hpp"
#include "Sample.hpp"
#include <string.h> 
#include <fstream>
#include <iostream>


class AudioController
{
private:
    
    float outputBlock[FRAMES_PER_BUFFER];
    float inputBlock[FRAMES_PER_BUFFER];
	bool recordThreadRunning;
    bool playThreadRunning;
	PaStream* inputStream;
    PaStream* outputStream;
	PaError error;
    PaStreamParameters outputParameters;
    PaStreamParameters inputParameters;
	std::fstream audioLog;
    BlockingQueue<Sample> bq;

public:

    AudioController(); 
    ~AudioController();
    void clearBuffers();
    void initializeAudio();
	void terminateAudio();
	void openStream();
	void closeStream();
	void startStream();
	void stopStream();
    void playAudio(BlockingQueue<Sample>& blockingQueue);
    void recordAudio(BlockingQueue<Sample>& blockingQueue);
    void catchPaError(const char* function);
    void catchPaErrorStream(const char* function);
    void stopPlayThread();
    void startPlayThread();
	void stopRecordThread();
    void startRecordThread();
    void test1()
    {
        float block[FRAMES_PER_BUFFER];
        Sample sample;
        for(int j = 0; j< 2500; j++)
        {
            //std::unique_lock<std::mutex> mlock(this->bqMutex);
            std::cout<<"nagrywam"<<j;
            Pa_ReadStream(inputStream, block, FRAMES_PER_BUFFER);
            sample.setSample(block);
            bq.push(sample);
           // mlock.unlock();
            
        }
    }
    void test2()
    {
        for(int i = 0; i < 2500; i++)
        {
            //std::unique_lock<std::mutex> mlock(this->bqMutex);
            std::cout<<"odtwarzam"<<i;
            Pa_WriteStream(outputStream, bq.pop().getSample(), FRAMES_PER_BUFFER);
            //mlock.unlock();
            
        }
    }
        // float* buffer;
        // buffer = (float*) malloc(sizeof(float)*500*FRAMES_PER_BUFFER);
        // for(int j = 0; j< 500; j++)
        // {
        //     std::cout<<"nagrywam"<<j;
        //     Pa_ReadStream(stream, buffer + j*FRAMES_PER_BUFFER, FRAMES_PER_BUFFER);
        // }
        // for(int i = 0; i < 500; i++)
        // {
        //     std::cout<<"odtwarzam"<<i;
        //     Pa_WriteStream(stream, buffer + i*FRAMES_PER_BUFFER, FRAMES_PER_BUFFER);
        // }
    
};

#endif