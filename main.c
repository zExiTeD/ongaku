#define MINIAUDIO_IMPLEMENTATION
#include "headers/miniaudio.h"


#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>

float volume = 0.2;


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if (pDecoder == NULL) {
    return;
  }
  ma_uint64 frameread; 
  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &frameread);
  //ma_device_set_master_volume(pDevice,volume);
  float* samples = (float*)pOutput;
  for (ma_uint64 i = 0; i < frameread * (pDevice->playback.channels); ++i) {
    samples[i] *= volume;
  }

  (void)pInput;
  
}


int kbhit(void)
{
  int k;

  ioctl(STDIN_FILENO,FIONREAD,&k);
  

  return(k);
}


int main(int argc, char** argv)
{
  ma_result result;
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;
  ma_uint64 pCursor;
  char choice ;

  char* files[3] = {
    "/home/yadu/Projects/MAIN/ongaku/music/noodles.mp3",
    "/home/yadu/Projects/MAIN/ongaku/music/DANDADAN-OP.mp3",
    "/home/yadu/Projects/MAIN/ongaku/music/song.mp3",
  };

  //if (argc < 2) {
  //  printf("No input file.\n");
  //  return -1;
  //  }
  for(int z = 0; z< 3 ;z++ ){
    result = ma_decoder_init_file(files[z], NULL, &decoder);
    if (result != MA_SUCCESS) {
      printf("Could not load file: %s\n", argv[1]);
      return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
      printf("Failed to open playback device.\n");
      ma_decoder_uninit(&decoder);
      return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
      printf("Failed to start playback device.\n");
      ma_device_uninit(&device);
      ma_decoder_uninit(&decoder);
      return -4;
    }

    ma_uint64 totalFrames , cursour;
    ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

    //printf("%lld \n ",totalFrames);

    printf("Press p to pause and r to resume \n");
    printf("Press b to go to back and n go to next \n");
    printf("Press l to increase volume and q to reduce volume \n");
    while(1){

      if (kbhit()) {

        choice = getchar();

        if ((choice) =='p' ) {
          printf("paused \n");
          ma_decoder_get_cursor_in_pcm_frames(&decoder, &pCursor);
          ma_device_stop(&device);
        }else if ((choice)=='r') {
          printf("resume \n");
          ma_decoder_seek_to_pcm_frame(&decoder, pCursor);
          ma_device_start(&device);
        }else if ((choice)=='l') {
          //  if (volume >=1.0) {
          //      continue;
          //  }
          volume += 0.2;
          printf(" volume %f  \n",volume);
        }else if ((choice) == 'q') {
          if (volume <= 0) {
            continue;
          }
          volume -=0.2;
          printf("volume %f  \n",volume);
        }else if ((choice) == 'n') {
          break;
        }else if ((choice) == 'b') {
          if (z==0) {
            continue;
          }
          z = z-2;
          break;
        }
      }
      ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursour);
      // printf("%lld \n",cursour);
      if (cursour==totalFrames) {
        printf("-- FInished -- \n");
        break;
      }

    }

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

  

    // check if there is any other music file in playlist?
  }
  return 0;
}
