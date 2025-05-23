/*
 * mp3_file.c
 *
 *  Created on: Jul 6, 2025
 *      Author: maciej
 */

#include "mp3_player.h"
#include "mp3dec.h"

#define BUFFER_SIZE 4096
#define MP3_SIZE	224958

static int16_t audio_buffer0[BUFFER_SIZE];
static int16_t audio_buffer1[BUFFER_SIZE];

static int buffer_number = 0;

static MP3FrameInfo mp3FrameInfo;
static HMP3Decoder hMP3Decoder;

I2S_HandleTypeDef *hi2s;

extern const char mp3_data[];

static bool mp3_player_prepare_buffer(int buffer)
{
	static const char *read_ptr = mp3_data;
	static int bytes_left = MP3_SIZE;

	int offset, err;
	bool result = true;

	int16_t *samples;

	if (buffer) {
		samples = audio_buffer0;
		// GPIO_SetBits(GPIOD, GPIO_Pin_13);
		// GPIO_ResetBits(GPIOD, GPIO_Pin_14);
	} else {
		samples = audio_buffer1;
		// GPIO_SetBits(GPIOD, GPIO_Pin_14);
		// GPIO_ResetBits(GPIOD, GPIO_Pin_13);
	}

	offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
	bytes_left -= offset;

	if (bytes_left <= 10000) {
		read_ptr = mp3_data;
		bytes_left = MP3_SIZE;
		offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
	}

	read_ptr += offset;
	err = MP3Decode(hMP3Decoder, (unsigned char**)&read_ptr, &bytes_left, samples, 0);

	if (err) {
		/* error occurred */
		switch (err) {
		case ERR_MP3_INDATA_UNDERFLOW:
			result = false;
			break;
		case ERR_MP3_MAINDATA_UNDERFLOW:
			/* do nothing - next call to decode will provide more mainData */
			break;
		case ERR_MP3_FREE_BITRATE_SYNC:
		default:
			result = false;
			break;
		}
	} else {
		/* no error */
		MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
	}

	return result;
}

static inline void mp3_player_push_stream(int buffer, int size)
{
	HAL_I2S_Transmit_DMA(hi2s, buffer ? audio_buffer0 : audio_buffer1, size);
}

bool mp3_player_init(I2S_HandleTypeDef *_hi2s)
{
	hi2s = _hi2s;
	hMP3Decoder = MP3InitDecoder();
	return hMP3Decoder != NULL;
}

bool mp3_player_start(void)
{
	const uint32_t start = HAL_GetTick();
	bool result = mp3_player_prepare_buffer(buffer_number);
	const uint32_t time = HAL_GetTick() - start;
	if (!result) {
		return false;
	}

	mp3_player_push_stream(buffer_number, mp3FrameInfo.outputSamps);

	buffer_number++;
	buffer_number %= 2;

	return true;
}

void mp3_player_end_stream_callback(void)
{
	mp3_player_push_stream(buffer_number, mp3FrameInfo.outputSamps);
	buffer_number++;
	buffer_number %= 2;
	mp3_player_prepare_buffer(buffer_number);
}
