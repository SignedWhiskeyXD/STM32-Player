#ifndef BUTTON_H
#define BUTTON_H

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_PLAY,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_NUM
} Button;

void initKeys();

void scanKeys();

#endif