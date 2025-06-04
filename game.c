

#include "game.h"
#include "timer_if.h"
extern Note notes[100];
extern Note notes_ms[100];

void convert_notes(Note* notes, Note* notes_ms) {
    int i = 0;
    while (notes_ms[i].lane != -1 && i < 100) {
        notes[i].delay = MILLISECONDS_TO_TICKS(notes_ms[i].delay);
        notes[i].lane = notes_ms[i].lane;
        i++;
    }

    // Mark end of list
    notes[i].delay = (unsigned long)-1;
    notes[i].lane = -1;  // Optional: sentinel value
}
