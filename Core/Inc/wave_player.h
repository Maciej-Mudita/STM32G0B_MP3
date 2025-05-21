/*
 * wave_player.h
 *
 *  Created on: 7 lip 2021
 *      Author: piotr
 */

#ifndef INC_WAVE_PLAYER_H_
#define INC_WAVE_PLAYER_H_

#include "stm32g0xx_hal.h"

#define FIRST_HALF_OF_BUFFER		1
#define SECOND_HALF_OF_BUFFER		2

void wave_player_init(I2S_HandleTypeDef *_hi2s);
void wave_player_start(uint8_t *file);
int8_t wave_player_read_header(uint8_t *file);
void wave_player_set_sample_rate(uint32_t sample_rate);
void wave_player_prepare_first_buffer(void);
void wave_player_prepare_half_buffer(uint8_t half_number);
void wave_player_prepare_data(uint32_t start_address, uint32_t end_address);

#endif /* INC_WAVE_PLAYER_H_ */
