#include "player.h"

#include <string.h>
#include "states/fileOps.h"
#include "vs1053/VS1053.h"

void playSelectedSong()
{
    File_State* fileState = useFileState();
    char musicFile[16] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFile + 3, fileState->filenames[selectedIndex]);

    vs1053_player_song(musicFile);
}