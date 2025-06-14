#include <stdint.h>
#ifndef __GAME_H__
#define __GAME_H__

#define DEFAULT_HIT_THRESHOLD_MS 375
#define DEFAULT_HIT_THRESHOLD 30000000
#define DEFAULT_SCREEN_THRESHOLD_MS 1000
#define MISS 0
#define HIT_EARLY 1
#define HIT_LATE 2


typedef struct {
    unsigned long delay;  // time(ms) until next note
    int8_t lane;   // lane number [0,1,2,3] [-1] indicates end of list
} Note;

void convert_notes(Note* notes, Note* notes_ms);
void InitGame();
void GameTapIntHandler();
//void TimerBaseA0IntHandler();



#endif
