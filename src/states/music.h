#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>

typedef struct {
    uint16_t avgByteRate;
    uint16_t decodeTime;
    uint32_t musicSize;
    int16_t offsetTime;
} MusicState;

MusicState* useMusicState();

void resetMusicState();

#endif