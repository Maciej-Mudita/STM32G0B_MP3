/*
 * wave_file.c
 *
 *  Created on: Jul 6, 2021
 *      Author: piotr
 */

#include "wave_player.h"
#include "main.h"
#include "wave_file.h"
#include <string.h>

#define AUDIO_BUFFER_SIZE			512
#define BYTES_IN_AUDIO_BUFFER_SIZE	AUDIO_BUFFER_SIZE*2

struct wave_player_s
{
	I2S_HandleTypeDef *hi2s;

    uint8_t *data_pointer;
	uint8_t buffer[AUDIO_BUFFER_SIZE];
	volatile uint32_t byte_counter;
	union wave_file_header file_hdr;
};

struct wave_player_s wave_player;
extern uint8_t audio_file[];

void wave_player_init(I2S_HandleTypeDef *_hi2s)
{
	wave_player.hi2s = _hi2s;
}

void wave_player_start(uint8_t *file)
{
	int8_t status;

	status = wave_player_read_header(file);

	if(ERROR == status)
	{
		return;
	}

	wave_player_set_sample_rate(wave_player.file_hdr.wave_file_hdr.sample_rate);
	wave_player_prepare_first_buffer();

	HAL_I2S_Transmit_DMA(wave_player.hi2s, (uint8_t *)wave_player.buffer, AUDIO_BUFFER_SIZE / 2);
}

int8_t wave_player_read_header(uint8_t *file)
{
	uint32_t i;

	wave_player.data_pointer = file;

	for(i = 0; i < WAVE_FILE_HEADER_SIZE; i++)
	{
		wave_player.file_hdr.bytes[i] = *(wave_player.data_pointer+i);
	}

	if(RIFF_HDR_CONST != wave_player.file_hdr.wave_file_hdr.riff_hdr)
	{
		return ERROR;
	}

	if(WAVE_ID_CONST != wave_player.file_hdr.wave_file_hdr.wave_id)
	{
		return ERROR;
	}

	if(FMT_CONST != wave_player.file_hdr.wave_file_hdr.chunk_marker)
	{
		return ERROR;
	}

	if(CHANNEL_MONO != wave_player.file_hdr.wave_file_hdr.channels)
	{
		return ERROR;
	}

	wave_player.byte_counter = WAVE_FILE_HEADER_SIZE;

	return SUCCESS;
}

void wave_player_set_sample_rate(uint32_t sample_rate)
{
	// wave_player.hi2s->Init.AudioFrequency = sample_rate;

	// HAL_SAI_InitProtocol(wave_player.hsai, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, 2);
}

void wave_player_prepare_first_buffer(void)
{
	wave_player_prepare_data(0, AUDIO_BUFFER_SIZE);
}

void wave_player_prepare_half_buffer(uint8_t half_number)
{
	if(FIRST_HALF_OF_BUFFER == half_number)
	{
		wave_player_prepare_data(0, AUDIO_BUFFER_SIZE/2);
	}
	else if(SECOND_HALF_OF_BUFFER == half_number)
	{
		HAL_I2S_Transmit_DMA(wave_player.hi2s, (uint8_t *)wave_player.buffer, AUDIO_BUFFER_SIZE / 2);
		wave_player_prepare_data(AUDIO_BUFFER_SIZE/2, AUDIO_BUFFER_SIZE);
	}
}

void wave_player_prepare_data(uint32_t start_address, uint32_t end_address)
{
	const uint32_t size = end_address - start_address;

	if(wave_player.byte_counter + size >= 450000) {
		wave_player.byte_counter = 0;
	}

	memcpy(&wave_player.buffer[start_address], wave_player.data_pointer + wave_player.byte_counter, size);
	wave_player.byte_counter += size;
}
