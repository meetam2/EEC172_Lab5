
//*****************************************************************************
//
// Application Name     -   RhythmDance
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup ssl
//! @{
//
//*****************************************************************************

//*****************************************************************************
// Pin Connections
//   MOSI (SI)  P7 on P2 header
//   SCK (CL)   P5 on P1 header
//   DC         P62 on P1 header
//   RESET (R)  P59 on P1 header
//   OLEDCS (OC) P61 on P1
//   SDCS (SC)  n.c. (no connection)
//   MISO (SO)  n.c. (no connection)
//   CD         n.c. (no connection)
//   3V         n.c. (no connection)
//   Vin (+)    3.3V
//   GND (G)    GND
//  IR SENSOR   P63 on p3
//  GPIO        P8 on p2
//
//  (INTERNAL)
//  SW2     P15 GPIOA2_BASE, .pin = 0x40
//  SW3     P4  GPIOA1_BASE, .pin = 0x20
//
//  I2c
//   SCL        P3
//   SDA        P6
//
//
//*****************************************************************************

//*****************************************************************************
// Timer 0 base: TIMERA0_BASE

#include <stdio.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_nvic.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"

//Common interface includes
#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"
#include "glcdfont.h"
#include "oled_test.h"
#include "pin_mux_config.h"
#include "gpio_if.h"
#include "gpio.h"
#include "common.h"
#include "uart_if.h"
#include "systick.h"
#include "timer.h"
#include "timer_if.h"
//#include "Adafruit_EX.c"

// Custom includes
//#include "sl_mqtt_client.h"

#include "utils/network_utils.h"
#include "network_helper.h"
#include "game.h"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

int iSockID = -1;
int noteIndex = 0;

Note notes_ms[100];
Note notes[100];

//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void BoardInit(void);
static int http_post(int);

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void) {
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void TimerBaseA0IntHandler(){
    unsigned long ulInts;
        ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
        //
        // Clear the timer interrupt.
        //
        MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    Report("Timer wraped. Note: %i\n", noteIndex);
    noteIndex++;
    if(notes[noteIndex].lane == -1){
        noteIndex = 0;
    }

//    Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, notes_ms[noteIndex].delay);
    TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[noteIndex].delay);
}

//*****************************************************************************
//
//! Main 
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void main() {
    //
    // Initialize board configuration
    //
    BoardInit();

    PinMuxConfig();

    InitTerm();
    ClearTerm();
    UART_PRINT("My terminal works!\n\r");

//    iSockID = initialize_network();
//    http_post(iSockID);
//
//    sl_Stop(SL_STOP_TIMEOUT);
//    LOOP_FOREVER();

    Note temp[] = {
        {2500, 1},
        {3000, 2},
        {2500, 3},
        {2000, 2},
        {-1, -1}
    };

    memcpy(notes_ms, temp, sizeof(temp));
    convert_notes(notes, temp);
//    Note notes[100] = {{2500, 1}, {3000, 2}, {2500, 3}, {2000,2}, {-1,-1}};

    //
    // Configuring the timer
    //
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerBaseA0IntHandler);

    GPIOInit();
    //
    // Turn on the timers feeding values in mSec
    //
//    Timer_IF_Start(TIMERA0_BASE, TIMER_A, notes_ms[0].delay);
    Report("tickks: %u %u %u\n", notes[0].delay, notes[1].delay, notes[2].delay);
    TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[0].delay);
    TimerEnable(TIMERA0_BASE, TIMER_A);

    //
    // Loop forever while the timers run.
    //
    while(1)
    {
    }

}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


