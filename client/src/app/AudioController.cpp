#include "AudioController.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AudioController::AudioController()
{
    int numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE;
    this->sampleBlock = (char*) malloc(numBytes);
    if( sampleBlock == NULL )
    {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    clearBuffer(this->sampleBlock, numBytes);


}

AudioController::~AudioController()
{
    free(sampleBlock);

}

void AudioController::initializeAudio()
{
    this->error = Pa_Initialize();
    if(this->error != paNoError){puts("chuuj0"); catchPa_Error();}

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

   if(error != paNoError){puts("chuuj1"); catchPa_Error();}
   
    fprintf( stdout, "Status: %s\n", Pa_GetErrorText(Pa_IsStreamActive(stream)) );

    //puts("dupa");
    for( int i=0; i<(5*SAMPLE_RATE)/FRAMES_PER_BUFFER; ++i )
    {
      /*******MIC INPUT*******/
      error = Pa_ReadStream( stream, sampleBlock, FRAMES_PER_BUFFER ); 
      // puts("dupa2");

      /*******SPEAKER OUTPUT*******/
      error = Pa_WriteStream( stream, sampleBlock, FRAMES_PER_BUFFER );
//puts("dupa3");
    }

    return;
}

void AudioController::closeAudio()
{
    error = Pa_StopStream( stream );
    fprintf( stdout, "Status: %s\n", Pa_GetErrorText(Pa_IsStreamActive(stream)) );
    if( error != paNoError ) {puts("chuuj2"); catchPa_Error();}
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