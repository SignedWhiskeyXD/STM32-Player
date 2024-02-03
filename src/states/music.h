#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>

typedef struct {
    uint16_t avgBitrate;
    uint16_t decodeTime;
    uint32_t musicSize;
} MusicState;

MusicState* useMusicState();

void resetMusicState();

#endif