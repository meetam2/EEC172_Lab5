
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
unsigned long hitThreshold = MILLISECONDS_TO_TICKS(DEFAULT_HIT_THRESHOLD_MS);
unsigned long screenThreshold = MILLISECONDS_TO_TICKS(DEFAULT_SCREEN_THRESHOLD_MS);

char MISSCC[5] = "MISS";
char HITCC[4] = "HIT";

// Arrow coordinates
volatile int y = 0;
int x = 1;
volatile int hit = -1;

Note notes_ms[100]; // Save Note.delay in ms
Note notes[100];    // Save Note.delay in ticks
Note results[100];   // Note.delay=|ticks| from note, Note.lane=[0,1,2] if MISS, HIT early, or HIT late

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

/**
 * Initializes SPI
 */
static void SPIInit(void) {
    MAP_SPIReset(GSPI_BASE);
    MAP_SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           8000000, SPI_MODE_MASTER, SPI_SUB_MODE_0,
                           (SPI_SW_CTRL_CS |
                            SPI_4PIN_MODE |
                            SPI_TURBO_OFF |
                            SPI_CS_ACTIVELOW |    // most SSD1351 modules use ACTIVELOW
                            SPI_WL_8));

    MAP_SPIEnable(GSPI_BASE);

}

void TimerBaseA0IntHandler(){
    unsigned long ulInts;
        ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
        //
        // Clear the timer interrupt.
        //
        MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    Report("Timer wraped. Note: %i\t Lane: %i\n", noteIndex, notes[noteIndex].lane);
    y = 0;
    noteIndex++;
    if(notes[noteIndex].lane == -1){
        //Game end
        results[noteIndex].lane = -1;
        results[noteIndex].delay = -1;
        noteIndex = 0;
    }

    // Load delay of next note
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

    SPIInit();
    //
    // Initialize Adafruit OLED
    //
    Adafruit_Init();
    fillScreen(BLACK);
    fillCircle(WIDTH/2, HEIGHT/2, 10, RED); //test screen

    // Initialize game variables
    InitGame();

    // Initialize timer
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
    Report("tickks: %u %u %u \tScreen threshold: %u\n", notes[0].delay, notes[1].delay, notes[2].delay, screenThreshold);
    TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[0].delay);
    TimerEnable(TIMERA0_BASE, TIMER_A);

//    int d = 90;
//        int y0 = -3*d;
//        int y1 = -4*d;
//        int y2 = 0;
//        int y3 = -2*d;
//        int bot = 97;
        while(1){
            //coverNote(0, &y0);
//            int d = drawLeft(1, &y0, MAGENTA);
//            drawUp(33, &y1, GREEN);
//            drawDown(65, &y2, BLUE);
//            drawRight(97, &y3, RED);
//            MAP_UtilsDelay(400000);
//            fillRect(1, y0, 31, 31, BLACK);
//            fillRect(33, y1, 31, 31, BLACK);
//            fillRect(65, y2, 31, 31, BLACK);
//            fillRect(97, y3, 31, 31, BLACK);
//
//            drawLeft(1, &bot, MAGENTA);
//            drawUp(33, &bot, GREEN);
//            drawDown(65, &bot, BLUE);
//            drawRight(97, &bot, RED);
//            y0+=4;
//            y1+=4;
//            y2+=4;
//            y3+=4;
            unsigned long time = TimerValueGet(TIMERA0_BASE, TIMER_A);   // Time until next note
//            Report("TIME: %u\t", time);
//            long time_diff = notes[noteIndex].delay - time;

            if(time < screenThreshold){
//                int y_new = 98 - ((double)(time * 98) / (double)(screenThreshold));
                int y_new = (98.0f - (((double)time*98) / (float)screenThreshold));
//                int y_new = ((screenThreshold - time) * 98) / screenThreshold;

                if (y_new == y){continue;}
                else if(y_new == 97){
//                    fillRect(x, 97, 31, 31, BLACK);
                    continue;
                }
                x = notes[noteIndex].lane * 32 + 1;
                fillRect(x, y, 31, 31, BLACK);
//                Report(" Display y: %d\tPercentage: %u\t", y, ((time * 98) / screenThreshold));
                Report("y: %i\t", y_new);
//                drawLeft(1, y_new, MAGENTA);
                drawArrow(x, y_new, notes[noteIndex].lane);
                y = y_new;
            }


            char *text;
                int xc = 52;
                if(hit){
                    text = HITCC;
                    xc = 64-9*2;
                    hit = -1;
                } else{
                    text = MISSCC;
                    xc = 64-12*2;
                    hit = -1;
                }
                int i = 0;
                for (i = 0; text[i] != '\0'; i++) {
                            drawChar(xc, 64, text[i], WHITE, RED, 2);
                            xc += 12; // Advance x for next character (adjust spacing as needed)
                }


        }
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


