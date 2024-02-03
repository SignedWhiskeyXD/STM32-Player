#include "music.h"

MusicState musicState;

MusicState* useMusicState()
{
    return &musicState;
}

void resetMusicState()
{
    musicState.avgBitrate = 0;
    musicState.decodeTime = 0;
    musicState.musicSize = 0;
}
