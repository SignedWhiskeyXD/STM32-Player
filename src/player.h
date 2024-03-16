#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#define BUFSIZE 4096

void    playSelectedSong();

uint8_t pauseOrResumeSelectedSong();

void    setJumpFlag(int8_t direction);

void    cancelPlayerTask();

#endif