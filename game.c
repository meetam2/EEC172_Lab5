
#include <stdio.h>
#include "hw_types.h"
#include "hw_memmap.h"
#include "game.h"
#include "prcm.h"
#include "timer.h"
#include "timer_if.h"

#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"
#include "glcdfont.h"
#include "oled_test.h"
extern Note notes[100];
extern Note notes_ms[100];
extern Note results[100];
extern volatile int hit;
extern int noteIndex;
extern unsigned long hitThreshold;
static unsigned long port;
static unsigned int pin;

char MISSC[5] = "MISS";
char HITC[4] = "HIT";


void InitGame(){
    int i = 0;
        for (i = 0; i < 100; i++) {
            results[i].delay = (unsigned long)-1;  // same as 0xFFFFFFFF
            results[i].lane = -1;                  // -1 = miss
        }

        Note temp[] = {
            {2500, 1},
            {2500, 2},
            {2500, 3},
            {2500, 2},
            {1500, 0},
            {1000, 2},
            {-1, -1}
        };

        memcpy(notes_ms, temp, sizeof(temp));
        convert_notes(notes, temp);
}

void SetGameTapConfig(unsigned long po, unsigned int pi){
    port = po;
    pin = pi;
}



void GameTapIntHandler(){
    unsigned long ulstatus = GPIOIntStatus(port, true);
    GPIOIntClear(port, ulstatus);

    // Disable interrupt
    if (ulstatus & pin){
//        MAP_TimerEnable(TIMERA0_BASE, TIMER_A);   // Start debounce timer
//                MAP_GPIOIntDisable(GPIOA1_BASE, GPIO_PIN_5);  // Disable button interrupt
        GPIOIntDisable(port, pin);
    }

    unsigned int time = TimerValueGet(TIMERA0_BASE, TIMER_A);   // Time until next note
    long time_diff = notes[noteIndex].delay - time;             // Time after prev note

    hit = 0;
    if(time <= hitThreshold){    // Note HIT early
        results[noteIndex].lane = HIT_EARLY;
        hit = HIT_EARLY;
        results[noteIndex].delay = time;
    }
    else if(time_diff <= hitThreshold){  // Note HIT late
        results[noteIndex].lane = HIT_LATE;
        hit = HIT_LATE;
        results[noteIndex].delay = time_diff;
    }


//    char *text;
//    int xc = 52;
//    if(hit){
//        text = HITC;
//        xc = 64-9*2;
//
//    } else{
//        text = MISSC;
//        xc = 64-12*2;
//    }
//    int i = 0;
//    for (i = 0; text[i] != '\0'; i++) {
//                drawChar(xc, 64, text[i], WHITE, RED, 2);
//                xc += 12; // Advance x for next character (adjust spacing as needed)
//            }
    Report("SW2 pressed. Timer value: %u \tDifference: %u \tHIT: %i\n", time, time_diff, hit);
    // Enable interrupt
    GPIOIntEnable(port, pin);
    return;
}

//void InitGameTimer(){
//    // Initialize timer
//    //
//    // Configuring the timer
//    //
//    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
//
//    //
//    // Setup the interrupts for the timer timeouts.
//    //
//    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerBaseA0IntHandler);
//
//}

//void TimerBaseA0IntHandler(){
//    unsigned long ulInts;
//        ulInts = TimerIntStatus(TIMERA0_BASE, true);
//        //
//        // Clear the timer interrupt.
//        //
//        TimerIntStatus(TIMERA0_BASE, ulInts);
//
//    Report("Timer wrapped. Note: %i\t Lane: %i\n", noteIndex, notes[noteIndex].lane);
//    noteIndex++;
//    if(notes[noteIndex].lane == -1){
//        //Game end
//        results[noteIndex].lane = -1;
//        results[noteIndex].delay = -1;
//        noteIndex = 0;
//    }
//
//    // Load delay of next note
////    Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, notes_ms[noteIndex].delay);
//    TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[noteIndex].delay);
//}

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
