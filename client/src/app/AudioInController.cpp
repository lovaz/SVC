#include "AudioInController.hpp"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>



AudioInController::AudioInController()
{
    int numBytes = FRAMES_PER_BUFFER;
    clearBuffer(this->sampleBlock, numBytes);
    stream = NULL;
    threadRunning = false;
    audioLog.open("audio_input.log", std::ios::out | std::ios::trunc);
    if(audioLog.good() != true)
    {
      std::cout << "Nie udało się otworzyć pliku audio_log"<<std::endl;
    }
}

AudioInController::~AudioInController()
{
    free(sampleBlock);
    if(audioLog.good() == true)
    {
      audioLog.close();
    }
}

void AudioInController::initializeAudio()
{
    this->error = Pa_Initialize();
    if(this->error != paNoError)
    {
      catchPaError();
    }

    inputParameters.device = Pa_GetDefaultInputDevice();
    if(audioLog.good() == true)
    {
      audioLog<<"******initializeAudio: input parameters******"<<std::endl;
      audioLog<<"Input device # "<<inputParameters.device<<std::endl;
      audioLog<<"Input LL: "<<Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency<<" s"<<std::endl;
      audioLog<<"Input HL: "<<Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency<<" s"<<std::endl;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    return;
}   


void AudioInController::openStream()
{
    this->error = Pa_OpenStream(
              &this->stream,
              &this->inputParameters,
              NULL, /* no output */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

    if(this->error != paNoError)
    { 
      catchPaError();
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******openStream******"<<std::endl;
      audioLog<<"Status: "<<Pa_GetErrorText(Pa_IsStreamActive(stream))<<std::endl;
    }

    this->error = Pa_StartStream( stream );

    if(this->error != paNoError) 
    {
      catchPaError();
    }
}

void AudioInController::recordAudio(BlockingQueue<Sample> &blockingQueue)
{
    Sample testSample;
    while(threadRunning)
    {
      this->error = Pa_ReadStream(stream, sampleBlock, FRAMES_PER_BUFFER);
      if(this->error != paNoError) 
      {
        catchPaErrorRec();
      }       
      testSample.setSample(sampleBlock);
      blockingQueue.push(testSample);
    }
    return;
}


void AudioInController::closeAudio()
{
    this->error = Pa_StopStream(stream);
    if(audioLog.good() == true)
    {
      audioLog<<"******closeAudio******"<<std::endl;
      audioLog<<"Status: "<<Pa_GetErrorText(Pa_IsStreamActive(stream))<<std::endl;
    }
    if(error != paNoError) 
    {
      catchPaError();
    }
    clearBuffer(sampleBlock, FRAMES_PER_BUFFER);
    free(sampleBlock);
    Pa_Terminate();
    return;
}



void AudioInController::clearBuffer(float* buf, int size)
{
    for(int i=0;i<size ;i++)
    {
        this->sampleBlock[i]= SAMPLE_SILENCE;
    }
    return;
}

void AudioInController::catchPaError()
{
    if(stream) 
    {
       Pa_AbortStream(stream);
       Pa_CloseStream(stream);
    }

    free(sampleBlock);
    Pa_Terminate();

    if(audioLog.good() == true)
    {
      audioLog<<"******caught error!******"<<std::endl;
      audioLog<<"An error occured while using the portaudio stream"<<std::endl;
      audioLog<<"Error number: "<<error<<std::endl;
      audioLog<<"Error message: "<<Pa_GetErrorText(error)<<std::endl;
    }

    if(audioLog.good() == true)
    {
      audioLog.close();
    }

    exit(-1);
}

void AudioInController::catchPaErrorRec()
{
    if(audioLog.good() == true)
    {
      audioLog<<"******caught recorder error!******"<<std::endl;
      audioLog<<"An error occured while using the portaudio stream"<<std::endl;
      audioLog<<"Error number: "<<error<<std::endl;
      audioLog<<"Error message: "<<Pa_GetErrorText(error)<<std::endl;
    }
    return;
}

void AudioInController::stopThread()
{
    threadRunning = false;
}

void AudioInController::startThread()
{
    threadRunning = true;
}