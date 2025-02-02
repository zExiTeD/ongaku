#include <string.h>
#define MINIAUDIO_IMPLEMENTATION
#include "headers/miniaudio.h"

#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

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

    if (argc < 2) {
        printf("No input file.\n");
        return -1;
    }

    result = ma_decoder_init_file(argv[1], NULL, &decoder);
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
    
    printf("Press p to pause and r to resume \n");
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
	    }	
	}
//	printf("\e[1;1H\e[2J");
    }

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}
