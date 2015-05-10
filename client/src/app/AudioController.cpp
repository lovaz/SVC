#include "AudioController.hpp"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

AudioController::AudioController()
{
    int numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE;
    this->sampleBlock = (char*) malloc(numBytes);
    this->record = (char*) malloc(numBytes*500);
    if( sampleBlock == NULL )
    {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    clearBuffer(this->sampleBlock, numBytes);
    stream = NULL;


}

AudioController::~AudioController()
{
    free(sampleBlock);
}

void AudioController::initializeAudio()
{
    this->error = Pa_Initialize();
    if(this->error != paNoError){puts("test0"); catchPa_Error();}

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    printf( "Input device # %d.\n", inputParameters.device );
    printf( "Input LL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency );
    printf( "Input HL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency );
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    printf( "Output device # %d.\n", outputParameters.device );
    printf( "Output LL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );
    printf( "Output HL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency );
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;



   return;
}   

void AudioController::IOAudioFunction()
{
        this->error = Pa_OpenStream(
              &this->stream,
              &this->inputParameters,
              &this->outputParameters,
              SAMPLE_RATE,
              0,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

   if(error != paNoError){puts("test1"); catchPa_Error();}
   
    fprintf( stdout, "Status: %s\n", Pa_GetErrorText(Pa_IsStreamActive(stream)) );

    error = Pa_StartStream( stream );
    if( error != paNoError ) catchPa_Error();

    for(int i=0; i<100; i++)
    {
       error = Pa_ReadStream( stream, record+(i*FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE), FRAMES_PER_BUFFER );
       if(error != paNoError){puts("testA"); catchPa_Error();}
       
    }
    for (int j = 0; j< 102400; ++j)
       {
         printf("Value: %d\n", *(record+j));     
       }

    for(int i=0; i<100; i++)
    {
        error = Pa_WriteStream( stream, record+(i*FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE), FRAMES_PER_BUFFER );
        if(error != paNoError){puts("testB"); catchPa_Error();}
    }



    return;
}

void AudioController::openStream()
{
        this->error = Pa_OpenStream(
              &this->stream,
              &this->inputParameters,
              &this->outputParameters,
              SAMPLE_RATE,
              0,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

    if(error != paNoError){puts("test1"); catchPa_Error();}
   
    fprintf( stdout, "Status: %s\n", Pa_GetErrorText(Pa_IsStreamActive(stream)) );

    error = Pa_StartStream( stream );
    if( error != paNoError ) catchPa_Error();
}

void AudioController::recordAudio(BlockingQueue<Sample> &blockingQueue)
{
     Sample testSample;

    for(int i=0; i<300; i++)
    {
       error = Pa_ReadStream( stream, record+(FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE), FRAMES_PER_BUFFER );
       if(error != paNoError){puts("testA"); catchPa_Error();}
       testSample.setSample(record+(FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE));
       blockingQueue.push(testSample);
    }
    return;
}

void AudioController::playAudio(BlockingQueue<Sample> &blockingQueue)
{
  sleep(2);
    for(int i=0; i<300; i++)
    {
        error = Pa_WriteStream( stream, blockingQueue.pop().getSample(), FRAMES_PER_BUFFER );
        if(error != paNoError){puts("testB"); catchPa_Error();}
    }
    return;
}


void AudioController::closeAudio()
{
    error = Pa_StopStream( stream );
    fprintf( stdout, "Status: %s\n", Pa_GetErrorText(Pa_IsStreamActive(stream)) );
    if( error != paNoError ) {puts("test2"); catchPa_Error();}
    clearBuffer(sampleBlock, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE);
    free( sampleBlock );
    Pa_Terminate();

    return;
}



void AudioController::clearBuffer(char* buf, int size)
{
    for(int i=0;i<size ;i++)
    {
        this->sampleBlock[i]= SAMPLE_SILENCE;
    }
    return;
}

void AudioController::catchPa_Error()
{
    if( stream ) {
       Pa_AbortStream( stream );
       Pa_CloseStream( stream );
    }
    free( sampleBlock );
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", error );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error ) );
    exit(-666);
    
}