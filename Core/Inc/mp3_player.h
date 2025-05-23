/*
 * mp3_player.h
 *
 *  Created on: 7 lip 2025
 *      Author: maciej
 */

#ifndef INC_MP3_PLAYER_H_
#define INC_MP3_PLAYER_H_

#include "stm32g0xx_hal.h"
#include <stdbool.h>

bool mp3_player_init(I2S_HandleTypeDef *_hi2s);
bool mp3_player_start(void);
void mp3_player_end_stream_callback(void);

#endif /* INC_MP3_PLAYER_H_ */
