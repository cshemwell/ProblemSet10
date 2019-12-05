#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <portaudio.h>
#include <sndfile.h>

//Compile with:
//gcc ProblemSet10.1.c -o 10.1 -lsndfile -lportaudio
//Run with:
//./10.1
//------------------------------------------------------------------------------------
//Constants
#define kInputFileName "12STRG40MIC.aif"
#define kDeviceIndex 1 //Built-in Output
#define kNumFramesPerBuffer 512
//------------------------------------------------------------------------------------
//Function prototypes
int openInputSndFile();
int initPortAudio();
int closePortAudio();
void printPaDevices();
//------------------------------------------------------------------------------------
//Audio render callback function
int renderCallback(
  const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData 
);
//------------------------------------------------------------------------------------
//Declare user data that holds SNDFILE and SF_INFO
//so that we can use them inside audio render callback
typedef struct SoundFile {
  SNDFILE *file;
  SF_INFO *info;
} SoundFile;
//------------------------------------------------------------------------------------
//Global variables
SNDFILE *gInFile = NULL; //pointers to a sound files
SF_INFO gSfInfo;
float* outBuffer;
//------------------------------------------------------------------------------------
int main(int argc, char *argv[]){
  PaStream *pStream = NULL; //For port audio streaming
	PaStreamParameters outputParameters; //Parameters for output of a stream
  SoundFile sndFile; //Used for audio render callback

  
  //Open sound file for a playback
  openInputSndFile();
  
  //Initialize port audio
  if(initPortAudio()) return 1;

  //Print available audio devices
  printPaDevices();

  //Configure port audio streaming setup
  memset(&outputParameters, 0, sizeof(PaStreamParameters));
  outputParameters.channelCount = gSfInfo.channels;
	outputParameters.device = kDeviceIndex;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = 0.0;
  
  //Copy sndfile so that we can access them inside the render callback function
  memset(&sndFile,0,sizeof(SoundFile));
  sndFile.file = gInFile;
  sndFile.info = &gSfInfo;

  //Open port audio streaming
  PaError error = Pa_OpenStream(
    &pStream,
    NULL, //no input
    &outputParameters, //output
    gSfInfo.samplerate,
    kNumFramesPerBuffer, //frames per buffer
    paNoFlag,
    renderCallback,
    &sndFile //User data
  );

  //Check for error in opening port audio streaming
  if(error != paNoError){
    printf("Error: Failed to open port audio stream. %s\n",Pa_GetErrorText(error));
    closePortAudio();
    return 1;
  }

  //Start port audio streaming
  error = Pa_StartStream(pStream);
  if(error != paNoError){
    printf("Error: Failed to start port audio stream. %s\n",Pa_GetErrorText(error));
  }
  else {
    printf("Rendering audio... Press any key to stop streaming audio\n");
    getchar();
    error = Pa_StopStream(pStream);
    if(error != paNoError){
      printf("Error: Failed to stop port audio stream. %s", Pa_GetErrorText(error));
    }
  }

  //Close port audio streaming
  if(closePortAudio()) return 1;

  //Clean up
  sf_close(gInFile);

  return 0;
}
//------------------------------------------------------------------------------------
int renderCallback(
  const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData 
){
  //Convert user data so that we can use it in the callback
  SoundFile *sndFile = (SoundFile *) userData;
  SNDFILE *file = sndFile->file;
  SF_INFO *info = sndFile->info;
  sf_count_t count;

  //Set output buffer to zero
  memset(output,0,frameCount*info->channels * sizeof(float));

  //Copy file content to output buffer frame count at a time
  count = sf_read_float(file, (float *) output,frameCount * info->channels);
//   for(int i = gSfInfo.frames; i )

  //Loop
  if(count != frameCount * info->channels){
    sf_seek(file,0,SEEK_SET);
  }

  return 0;
}
//------------------------------------------------------------------------------------
int openInputSndFile(){
  //Initialize SF_INFO with 0s (Required for reading)
  memset(&gSfInfo, 0, sizeof(SF_INFO));

  //Open the original sound file as read mode
  gInFile = sf_open(kInputFileName, SFM_READ, &gSfInfo);
  if(!gInFile){//Check if the file was succefully opened
    printf("Error : could not open file : %s\n", kInputFileName);
		puts(sf_strerror(NULL));
		return 1;
  }

  //Check if the file format is in good shape
  if(!sf_format_check(&gSfInfo)){	
    sf_close(gInFile);
		printf("Invalid encoding\n");
		return 1;
	}

  //Check if the file is mono
  if(gSfInfo.channels > 1){
    printf("Input file is not mono\n");
    return 1;
  }

  //print out information about opened sound file
  printf("Sample rate for this file is %d\n", gSfInfo.samplerate);
  printf("A number of channels in this file is %d\n", gSfInfo.channels);
  printf("A number of frames in this file is %lld\n", gSfInfo.frames);
  printf("Duration is %f\n", (double)gSfInfo.frames / gSfInfo.samplerate);

  outBuffer = malloc(gSfInfo.frames * sizeof(float));

  for(int i = 0; i < -1 * gSfInfo.frames; i--){
    sf_seek(gInFile, i, SEEK_END);
    sf_read_float(gInFile, outBuffer + (-i), 1);
  }



  return 0;
}
//------------------------------------------------------------------------------------
int initPortAudio(){ //Initialize Port Audio
  PaError error = Pa_Initialize();
  if(error != paNoError){
    printf("Error: Pa_Initialize() failed with %s\n", Pa_GetErrorText(error));
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------------
int closePortAudio(){ //Terminate Port Audio
  PaError error = Pa_Terminate();
  if(error != paNoError){
    printf("Error: Pa_Terminate() failed with %s\n",Pa_GetErrorText(error));
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------------
void printPaDevices(){
  //Get number of port audio devices available
  PaDeviceIndex numDevices = Pa_GetDeviceCount(); 
  PaDeviceIndex curDeviceID;

  const PaDeviceInfo *pDeviceInfo; 
  const PaHostApiInfo *pHostApiInfo;

  //Iterate over each device and print out information about them
  for(curDeviceID = 0; curDeviceID < numDevices; curDeviceID++){
    pDeviceInfo = Pa_GetDeviceInfo(curDeviceID);
    pHostApiInfo = Pa_GetHostApiInfo(pDeviceInfo->hostApi);
    printf("--------------------------------------------\n");
    printf("ID: %d, Name: %s, ", curDeviceID, pDeviceInfo->name);
    printf("API name: %s\n", pHostApiInfo->name);
    printf("Max output channels: %d\t", pDeviceInfo->maxOutputChannels);
    printf("Max input channels: %d\n\n", pDeviceInfo->maxInputChannels);
  }
}