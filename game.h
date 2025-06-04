#include <stdint.h>
#ifndef __GAME_H__
#define __GAME_H__
typedef struct {
    unsigned long delay;  // time(ms) until next note
    int8_t lane;   // lane number [1,2,3,4] [-1] indicates end of list
} Note;

#endif
