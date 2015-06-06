// SVC - Simple Voice Communicator 
// kontroler dźwięku
// autor: Mateusz Wolf


#include "AudioController.hpp"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>



AudioController::AudioController()
{
    clearBuffers();
    inputStream = NULL;
    outputStream = NULL;
    playThreadRunning = false;
    recordThreadRunning = false;
    audioLog.open("audio.log", std::ios::out | std::ios::trunc);
    if(audioLog.good() != true)
    {
      std::cout << "Nie udało się otworzyć pliku audio_log"<<std::endl;
    }
}

AudioController::~AudioController()
{
    if(audioLog.good() == true)
    {
      audioLog.close();
    }
}

void AudioController::initializeAudio()
{
    this->error = Pa_Initialize();
    if(this->error != paNoError)
    {
      catchPaError("initializeAudio");
    }

    inputParameters.device = Pa_GetDefaultInputDevice();
    if(audioLog.good() == true)
    {
      audioLog<<"******initializeAudio: input parameters******"<<std::endl;
      audioLog<<"Input device # "<<inputParameters.device<<std::endl;
      audioLog<<"Input LL: "<<Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency<<" s"<<std::endl;
      audioLog<<"Input HL: "<<Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency<<" s"<<std::endl;
      audioLog<<"******initializeAudio: input parameters DONE******"<<std::endl;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;
	
    outputParameters.device = Pa_GetDefaultOutputDevice(); 
    if(audioLog.good() == true)
    {
      audioLog<<"******initializeAudio: output parameters******"<<std::endl;
      audioLog<<"Output device # "<<outputParameters.device<<std::endl;
      audioLog<<"Output LL: "<<Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency<<" s"<<std::endl;
      audioLog<<"Output HL: "<<Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency<<" s"<<std::endl;
      audioLog<<"******initializeAudio: output parameters DONE******"<<std::endl;
    }
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    return;
}   

void AudioController::terminateAudio()
{
    Pa_Terminate();
    if(audioLog.good() == true)
    {
      audioLog<<"******terminateAudio******"<<std::endl;
      audioLog<<"Status: OK"<<std::endl;
    }
    return;
}


void AudioController::openStream()
{
    this->error = Pa_OpenStream(
              &this->inputStream,
              &this->inputParameters,
              NULL,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

    if(this->error != paNoError)
    { 
      catchPaError("openStream input");
    }
    if(audioLog.good() == true)
    {
		  audioLog<<"******openStream input******\n"<<"Status: OK"<<std::endl;
    }
    
    this->error = Pa_OpenStream(
              &this->outputStream,
              NULL,
              &this->outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

    if(this->error != paNoError)
    { 
      catchPaError("openStream output");
    }
    if(audioLog.good() == true)
    {
      audioLog<<"******openStream output******\n"<<"Status: OK"<<std::endl;
    }
    return;
}


void AudioController::closeStream()
{
    this->error = Pa_AbortStream(inputStream);
    if(error != paNoError && error != paStreamIsStopped) 
    {
      catchPaError("closeStream output");
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******closeAudio input******"<<std::endl;
      audioLog<<"Status: OK"<<std::endl;
    }

    this->error = Pa_AbortStream(outputStream);
    if(error != paNoError && error != paStreamIsStopped) 
    {
      catchPaError("closeStream output");
    }
    if(audioLog.good() == true)
    {
      audioLog<<"******closeAudio output******"<<std::endl;
      audioLog<<"Status: OK"<<std::endl;
    }
    clearBuffers();
    return;
}


void AudioController::startStream()
{
    this->error = Pa_StartStream( inputStream );
	
    if(this->error != paNoError) 
    {
      catchPaError("startStream input");
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******startStream input******"<<std::endl;
      audioLog<<"startStatusA: "<<Pa_IsStreamActive(inputStream)<<std::endl;
      audioLog<<"startStatusS: "<<Pa_IsStreamStopped(inputStream)<<std::endl;
    }
      
    this->error = Pa_StartStream( outputStream );
  
    if(this->error != paNoError) 
    {
      catchPaError("startStream output");
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******startStream output******"<<std::endl;
      audioLog<<"startStatusA: "<<Pa_IsStreamActive(outputStream)<<std::endl;
      audioLog<<"startStatusS: "<<Pa_IsStreamStopped(outputStream)<<std::endl;
    }
    fflush(stdout);
    return;
}


void AudioController::stopStream()
{
    this->error = Pa_StopStream( inputStream );

    if(this->error != paNoError) 
    {
      catchPaError("stopStream input");
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******stopStream input******"<<std::endl;
      audioLog<<"stopStatusA: "<<Pa_IsStreamActive(inputStream)<<std::endl;
      audioLog<<"stopStatusS: "<<Pa_IsStreamStopped(inputStream)<<std::endl;
    }

    this->error = Pa_StopStream( outputStream );

    if(this->error != paNoError) 
    {
      catchPaError("stopStream output");
    }

    if(audioLog.good() == true)
    {
      audioLog<<"******stopStream output******"<<std::endl;
      audioLog<<"stopStatusA: "<<Pa_IsStreamActive(outputStream)<<std::endl;
      audioLog<<"stopStatusS: "<<Pa_IsStreamStopped(outputStream)<<std::endl;
    }
    return;
}


void AudioController::recordAudio(BlockingQueue<Sample> &blockingQueue)
{
    Sample sample;
    while(recordThreadRunning)
    {
      this->error = Pa_ReadStream(inputStream, inputBlock, FRAMES_PER_BUFFER);
      if(this->error != paNoError) 
      {
        catchPaErrorStream("recorder");
      }       
      sample.setSample(inputBlock);
      blockingQueue.push(sample);
    }
    return;
}

void AudioController::playAudio(BlockingQueue<Sample> &blockingQueue)
{
    while(playThreadRunning)
    {
      this->error = Pa_WriteStream(outputStream, blockingQueue.pop().getSample(), FRAMES_PER_BUFFER);
      if(this->error != paNoError) 
      {
        catchPaErrorStream("player");
      }
    }
    return;
}


void AudioController::clearBuffers()
{
    for(int i=0;i<FRAMES_PER_BUFFER ;i++)
    {
        this->inputBlock[i]= SAMPLE_SILENCE;
		this->outputBlock[i]= SAMPLE_SILENCE;
    }
    return;
}

void AudioController::catchPaError(const char* function)
{
    if(inputStream) 
    {
       Pa_AbortStream(inputStream);
       Pa_CloseStream(inputStream);
    }
    if(outputStream) 
    {
       Pa_AbortStream(outputStream);
       Pa_CloseStream(outputStream);
    }

    Pa_Terminate();

    if(audioLog.good() == true)
    {
      audioLog<<"******caught error in: "<<function<<"!******"<<std::endl;
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

void AudioController::catchPaErrorStream(const char* function)
{
    if(audioLog.good() == true)
    {
      audioLog<<"******caught "<<function<<" error!******"<<std::endl;
      audioLog<<"An error occured while using the portaudio stream"<<std::endl;
      audioLog<<"Error number: "<<error<<std::endl;
      audioLog<<"Error message: "<<Pa_GetErrorText(error)<<std::endl;
    }
    return;
}

void AudioController::stopPlayThread()
{
    playThreadRunning = false;
}

void AudioController::startPlayThread()
{
    playThreadRunning = true;
}

void AudioController::stopRecordThread()
{
    recordThreadRunning = false;
}

void AudioController::startRecordThread()
{
    recordThreadRunning = true;
}