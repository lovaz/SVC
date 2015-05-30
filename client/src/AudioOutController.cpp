#include "AudioOutController.hpp"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>



AudioOutController::AudioOutController()
{
    int numBytes = FRAMES_PER_BUFFER;
    clearBuffer(this->sampleBlock, numBytes);
    stream = NULL;
    threadRunning = false;
    audioLog.open("audio_output.log", std::ios::out | std::ios::trunc);
    if(audioLog.good() != true)
    {
      std::cout << "Nie udało się otworzyć pliku audio_input.log"<<std::endl;
    }
}

AudioOutController::~AudioOutController()
{
    if(audioLog.good() == true)
    {
      audioLog.close();
    }
}

void AudioOutController::initializeAudio()
{
    this->error = Pa_Initialize();
    if(this->error != paNoError)
    {
      catchPaError();
    }

    outputParameters.device = Pa_GetDefaultOutputDevice(); 
    if(audioLog.good() == true)
    {
      audioLog<<"******initializeAudio: output parameters******"<<std::endl;
      audioLog<<"Output device # "<<outputParameters.device<<std::endl;
      audioLog<<"Output LL: "<<Pa_GetDeviceInfo(outputParameters.device)->defaultLowInputLatency<<" s"<<std::endl;
      audioLog<<"Output HL: "<<Pa_GetDeviceInfo(outputParameters.device)->defaultHighInputLatency<<" s"<<std::endl;
    }
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    return;
}   


void AudioOutController::openStream()
{
    this->error = Pa_OpenStream(
              &this->stream,
              NULL, /* no input */
              &this->outputParameters,
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


void AudioOutController::playAudio(BlockingQueue<Sample> &blockingQueue)
{
    while(threadRunning)
    {
      this->error = Pa_WriteStream(stream, blockingQueue.pop().getSample(), FRAMES_PER_BUFFER);
      if(this->error != paNoError) 
      {
        catchPaErrorPlay();
      }
    }
    return;
}


void AudioOutController::closeAudio()
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
   // Pa_Terminate();
    return;
}



void AudioOutController::clearBuffer(float* buf, int size)
{
    for(int i=0;i<size ;i++)
    {
        this->sampleBlock[i]= SAMPLE_SILENCE;
    }
    return;
}

void AudioOutController::catchPaError()
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


void AudioOutController::catchPaErrorPlay()
{
    if(audioLog.good() == true)
    {
      audioLog<<"******caught player error!******"<<std::endl;
      audioLog<<"An error occured while using the portaudio stream"<<std::endl;
      audioLog<<"Error number: "<<error<<std::endl;
      audioLog<<"Error message: "<<Pa_GetErrorText(error)<<std::endl;
    }
    return;
}

void AudioOutController::stopThread()
{
    threadRunning = false;
}

void AudioOutController::startThread()
{
    threadRunning = true;
}