
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

// Driverlib includes
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

// Common interface includes
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
// #include "Adafruit_EX.c"

// Custom includes
// #include "sl_mqtt_client.h"

#include "utils/network_utils.h"
#include "input_if.h"
#include "network_helper.h"
#include "game.h"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

#if defined(ccs) || defined(gcc)
extern void (*const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#define TICKS_TO_US(ticks)                   \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
     ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))

int iSockID = -1;
int noteIndex = 0;
unsigned long hitThreshold = MILLISECONDS_TO_TICKS(DEFAULT_HIT_THRESHOLD_MS);
unsigned long screenThreshold = MILLISECONDS_TO_TICKS(DEFAULT_SCREEN_THRESHOLD_MS);

char MISSCC[5] = "MISS";
char HITCC[4] = "HIT";
char user[4] = "ABC";

int INVERTY = 0;
int INVERTX = 0;

// Arrow coordinates
volatile int y = 0;
int x = 1;
volatile int hit = -1;
volatile int start = -1; // Game Start

Note notes_ms[100]; // Save Note.delay in ms
Note notes[100];    // Save Note.delay in ticks
Note results[100];  // Note.delay=|ticks| from note, Note.lane=[0,1,2] if MISS, HIT early, or HIT late

//*****************************************************************************

char letterSets[11][12] = {
    "            ", "", "ABCABCABCABC", "DEFDEFDEFDEF", "GHIGHIGHIGHI", "JKLJKLJKLJKL", "MNOMNOMNOMNO", "PQRSPQRSPQRS", "TUVTUVTUVTUV", "WXYZWXYZWXYZ", ""};
static const PinSetting IR = {.port = GPIOA1_BASE, .pin = 0x1}; // pin 63
const int LETTER_SIZE = 2;
const int MESSAGE_LENGTH = 32;
volatile int TXMsgIndex = 0;
volatile int RXMsgIndex = 0;
volatile char TXMsg[MESSAGE_LENGTH + 2];
volatile char RXMsg[MESSAGE_LENGTH + 2];
// the cc3200's fixed clock frequency of 80 MHz
#define SYSCLKFREQ 80000000ULL

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

#define START_BIT_CLKS 3070000UL // SYSTICK_RELOAD_VAL - Start bit pulse length in cycles

#define IR_LEN 48 // Minimum number of bits expected in each IR transmission
#define LETTER_SIZE 2

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;

// track if IR interrupt detected. Resets after transmission ended
volatile int IRFlag = 0;

// track if IR transmission finished
volatile int IRFinish = 0;
volatile int RXFinish = 0;
volatile int sendfinish = 0;

// track how many (falling) edges detected in one transmission
volatile int edgeCount = 0;

// Systick register value
unsigned long reg = 0;

// Buffer for pulse lengths
unsigned long buffer[100];

// Track the remote button pushed previously and currently
int currentButton = -2;
int prevButton = -1;
int buttonIndex = 0; // index for set of letters the currentButton corresponds to (e.g. {A,B,C} for button 1)

//*****************************************************************************

//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void BoardInit(void);
static int http_post(int);
// static void DisplayMessage();
static void DisplayMessageC(char *message, int yi);
static void DisplayMessageT(char *message);
/**
 * Reset SysTick Counter
 */
static inline void SysTickReset(void)
{
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void)
{
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
static void SPIInit(void)
{
    MAP_SPIReset(GSPI_BASE);
    MAP_SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           8000000, SPI_MODE_MASTER, SPI_SUB_MODE_0,
                           (SPI_SW_CTRL_CS |
                            SPI_4PIN_MODE |
                            SPI_TURBO_OFF |
                            SPI_CS_ACTIVELOW | // most SSD1351 modules use ACTIVELOW
                            SPI_WL_8));

    MAP_SPIEnable(GSPI_BASE);
}

//*****************************************************************************
//
//! Timer interrupt handler for game
//! Restarts y position for note, increments noteIndex, set timer load for next note, set flag for end of game
//
//*****************************************************************************
void TimerBaseA0IntHandler()
{
    unsigned long ulInts;
    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
    //
    // Clear the timer interrupt.
    //
    MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    Report("Timer wraped. Note: %i\t Lane: %i\n", noteIndex, notes[noteIndex].lane);
    y = 0;
    noteIndex++;
    if (notes[noteIndex].lane == -1)
    {
        // Game end
        start = 0; // Game end flag
        results[noteIndex].lane = -2;
        results[noteIndex].delay = -2;
        noteIndex = 0;
    }

    // Load delay of next note
    //    Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, notes_ms[noteIndex].delay);
    TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[noteIndex].delay);
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void)
{
    // increment every time the systick handler fires
    systick_cnt++;

    // no IR interrupts ~160ms -> assume IR transmission finished
    if (systick_cnt > 1)
    {
        if (edgeCount < 48)
        {
            // skip this IR transmission - too few bits
            edgeCount = 0;
            return;
        }
        if (TICKS_TO_US((SYSTICK_RELOAD_VAL - buffer[0]) / 800 != 2))
        {
            // skip this IR transmission - start bit is incorrect
            Report("Skipped: start bit incorrect\n");
            edgeCount = 0;
            return;
        }
        // more than 48 edges detected -> IR transmission is likely valid -> indicate that IR transmission is finished
        IRFinish = 1;
    }
    return;
}

/**
 * Interrupt handler for IR GPIO port
 *
 * Only used here for decoding IR transmissions
 */
static void IRIntHandler(void)
{

    // static int prev_state = 1;
    long read = GPIOPinRead(IR.port, IR.pin);
    int highIR = (read & IR.pin);

    // get and clear status
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus(IR.port, true);
    MAP_GPIOIntClear(IR.port, ulStatus);

    if (IRFinish)
    {
        return;
    }

    // rising -> reset to 0
    // falling get time delay
    //  check if interrupt occured on IR pin
    if (ulStatus & IR.pin)
    {
        if (!highIR)
        {
            ////Report("FALLING EDGE\n");
            // previous state was high -> falling edge

            if (!systick_cnt && edgeCount < 100)
            {
                // if systick expired, the pulse was longer than 40ms
                // don't measure it in that case

                // Get current Systick register value
                reg = HWREG(NVIC_ST_CURRENT);

                buffer[edgeCount] = reg;
                edgeCount++;
            }
        }
        else if (highIR)
        {
            ////Report("RISING EDGE\n");
            // previous state was low -> rising edge

            // begin measuring a new pulse width, reset the delta and systick
            SysTickReset();
        }
    }

    return;
}

/**
 * Executes command corresponding to currentButton
 *
 *
 */
static void ExecuteButton(void)
{
    // Button 1 does nothing
    if (currentButton == 1)
    {
    }
    else if (currentButton == 10)
    { // ENTER -> START GAME
        const char *message = "GAME START!!!";
        DisplayMessageC(message, 42);
        start = 1;
    }
    else if (currentButton == 11)
    { // DELETE
        // Set current letter to blank
        TXMsg[TXMsgIndex] = '\0';

        // Decrement TXMsg index
        if (TXMsgIndex > 0)
        {
            TXMsgIndex--;
        }
    }
    else if (currentButton == prevButton)
    {
        // Increment button index and set current letter
        buttonIndex++;
        TXMsg[TXMsgIndex] = letterSets[currentButton][buttonIndex % 12];
    }
    else if (currentButton != prevButton)
    {
        if (TXMsgIndex < MESSAGE_LENGTH - 1)
        { // Message length has not exceeded max length
            if (TXMsg[0] != '\0')
            { // if first letter is NULL, then index should not increment
                TXMsgIndex++;
            }
        }

        // Reset button index position and set current letter
        buttonIndex = 0;
        TXMsg[TXMsgIndex] = letterSets[currentButton][buttonIndex % 12];
    }

    //    DisplayMessage();
    prevButton = currentButton;
}

/**
 * Decode IR pulse delays to TV remote button then run ExecuteButton()
 *
 *
 */
static void DecodeButton(void)
{

    // iterate through number of edges, at most IR_LEN number of edges +2 (48 + 2)
    int i = 0;

    for (i = 0; i < 50; i++)
    {
        // Decode number of cycles to microseconds
        unsigned long delay = TICKS_TO_US(SYSTICK_RELOAD_VAL - buffer[i]);

        // Decode microseconds to binary (start bit is approximately 1700 and will be decoded as 2)
        int bit = delay / 800;
    }

    unsigned long long code = 0;

    for (i = 0; i < 49; i++)
    {
        unsigned long delay = TICKS_TO_US(SYSTICK_RELOAD_VAL - buffer[i]);
        int bit = delay > 1200 ? 1 : 0;
        code = (code << 1) | bit;
    }

    // Report("Decoded code: 0x%llx\n", code);

    if (code == BTN_ONE)
        Report("Button: 1\n");
    else if (code == BTN_TWO)
        Report("Button: 2\n");
    else if (code == BTN_THREE)
        Report("Button: 3\n");
    else if (code == BTN_FOUR)
        Report("Button: 4\n");
    else if (code == BTN_FIVE)
        Report("Button: 5\n");
    else if (code == BTN_SIX)
        Report("Button: 6\n");
    else if (code == BTN_SEVEN)
        Report("Button: 7\n");
    else if (code == BTN_EIGHT)
        Report("Button: 8\n");
    else if (code == BTN_NINE)
        Report("Button: 9\n");
    else if (code == BTN_ZERO)
        Report("Button: 0\n");
    else if (code == BTN_ENTER)
        Report("Button: ENTER\n");
    else if (code == BTN_DELETE)
        Report("Button: DELETE\n");
    else
        Report("Button: UNKNOWN\n");

    if (code == BTN_ONE)
        currentButton = 1;
    else if (code == BTN_TWO)
        currentButton = 2;
    else if (code == BTN_THREE)
        currentButton = 3;
    else if (code == BTN_FOUR)
        currentButton = 4;
    else if (code == BTN_FIVE)
        currentButton = 5;
    else if (code == BTN_SIX)
        currentButton = 6;
    else if (code == BTN_SEVEN)
        currentButton = 7;
    else if (code == BTN_EIGHT)
        currentButton = 8;
    else if (code == BTN_NINE)
        currentButton = 9;
    else if (code == BTN_ZERO)
        currentButton = 0;
    else if (code == BTN_ENTER)
        currentButton = 10;
    else if (code == BTN_DELETE)
        currentButton = 11;
    //    else Report("Button: UNKNOWN\n");

    ExecuteButton();
}

// static void DisplayMessage() {
//     char *message = "PRESS ENTER TO START";
//     int x = 0;
//     int y = 42;
//     int i = 0;
//
//     for (i = 0; message[i] != '\0'; i++) {
//         // Wrap to next line if x exceeds screen width
//         if (x + (6 * LETTER_SIZE) > 128 - LETTER_SIZE) {
//             x = 0;
//             y += 8 * LETTER_SIZE;
//         }
//
//         drawChar(x, y, message[i], RED, BLUE, LETTER_SIZE);
//         x += 6 * LETTER_SIZE;
//     }
// }

/*
 * Displays message, wraps around when x reaches edge of screen
 *
 * param    message - message to display
 *          yi - initial y position
 */
static void DisplayMessageC(char *message, int yi)
{
    //    int i = 0;
    //    while (message[i] != '\0') {
    //        drawChar(i * LETTER_SIZE * 6, 0, message[i], MAGENTA, GREEN, 2);
    //        i++;
    //    }
    int x = 0;
    int y = yi;
    int i = 0;

    for (i = 0; message[i] != '\0'; i++)
    {
        // Wrap to next line if x exceeds screen width
        if (x + (6 * LETTER_SIZE) > 128 - LETTER_SIZE)
        {
            x = 0;
            y += 8 * LETTER_SIZE;
        }

        drawChar(x, y, message[i], RED, BLUE, LETTER_SIZE);
        x += 6 * LETTER_SIZE;
    }
}

static void DisplayMessageT(char *message)
{
    //    int i = 0;
    //    while (message[i] != '\0') {
    //        drawChar(i * LETTER_SIZE * 6, 0, message[i], MAGENTA, GREEN, 2);
    //        i++;
    //    }
    int x = 0;
    int y = 0;
    int i = 0;

    for (i = 0; message[i] != '\0'; i++)
    {
        // Wrap to next line if x exceeds screen width
        if (x + (6 * LETTER_SIZE) > 128 - LETTER_SIZE)
        {
            x = 0;
            y += 8 * LETTER_SIZE;
        }

        drawChar(x, y, message[i], GREEN, BLUE, LETTER_SIZE);
        x += 6 * LETTER_SIZE;
    }
}

/**
 * Initializes GPIO Interrupts for IR sensor
 */
static void GPIOIRInit(void)
{
    GPIOIntRegister(IR.port, IRIntHandler);
    GPIOIntTypeSet(IR.port, IR.pin, GPIO_BOTH_EDGES);
    //    GPIOIntTypeSet(IR.port, IR.pin, GPIO_FALLING_EDGE);
    GPIOIntClear(IR.port, IR.pin);
    GPIOIntEnable(IR.port, IR.pin);
}

/**
 * Initializes SysTick Module
 */
static void SysTickInit(void)
{

    // configure the reset value for the systick countdown register
    SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    SysTickIntEnable();

    // enable the systick module itself
    SysTickEnable();
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
void main()
{
    char *message[50];
    //
    // Initialize board configuration
    //
    BoardInit();

    PinMuxConfig();

    InitTerm();
    ClearTerm();
    UART_PRINT("My terminal works!\n\r");

    // Enable SysTick
    SysTickInit();
    GPIOIRInit();

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
    start = 0;

    SPIInit();
    //
    // Initialize Adafruit OLED
    //
    Adafruit_Init();
    fillScreen(BLACK);
    fillCircle(WIDTH / 2, HEIGHT / 2, 10, RED); // test screen

    fillScreen(BLACK);

    // Initialize socket
    *message = "Connecting to AWS...";
    DisplayMessageC(message, 20);
    MAP_UtilsDelay(500000);
    iSockID = initialize_network();

    fillScreen(BLACK);
    *message = "Connected!";
    DisplayMessageC(message, 20);

    char jsonData[] = "\"default\": \"message \"";
    //    http_postmsg(iSockID, jsonData);

    while (1)
    {
        while (!start)
        {
            char *message = "PRESS ENTER TO START";
            DisplayMessageC(message, 42);
            if (IRFinish)
            {
                // IRSignal detected
                // Report("IR finished. Edge Count: %d\n", edgeCount);
                if (edgeCount < IR_LEN || (TICKS_TO_US(SYSTICK_RELOAD_VAL - buffer[0])) / 800 != 2)
                {
                    Report("Error: invalid IR reading\n");
                }
                else
                {
                    DecodeButton();
                }

                IRFinish = 0;
                edgeCount = 0;
            }
        }

        //
        // Turn on the timers feeding values in mSec
        //
        //    Timer_IF_Start(TIMERA0_BASE, TIMER_A, notes_ms[0].delay);
        TimerLoadSet(TIMERA0_BASE, TIMER_A, notes[0].delay);
        TimerEnable(TIMERA0_BASE, TIMER_A);
        Report("GAME START!!\n");
        char *message = ("GAME START!!");

        fillScreen(BLACK);
        DisplayMessageC(message, 42);
        drawLeft(1, 98, MAGENTA);
        drawDown(33, 98, BLUE);
        drawUp(65, 98, GREEN);
        drawRight(97, 98, RED);

        while (start)
        {
            unsigned long time = TimerValueGet(TIMERA0_BASE, TIMER_A); // Time until next note

            // Display note when note position is within screenThreshold
            if (time < screenThreshold)
            {
                int y_new = (98.0f - (((double)time * 98) / (float)screenThreshold));

                if (y_new == y)
                {
                    continue;
                }
                else if (y_new == 97)
                {
                    // fillRect(x, 97, 31, 31, BLACK);
                    continue;
                }
                x = notes[noteIndex].lane * 32 + 1;
                fillRect(x, y, 31, 31, BLACK);
                drawArrow(x, y_new, notes[noteIndex].lane);
                y = y_new;
            }

            // Display HIT or MISS when button pressed
            if (hit != -1)
            {
                // Display HIT or MISS
                char *text;
                int xc = 52;
                if (hit)
                {
                    text = HITCC;
                    xc = 64 - 9 * 2;
                }
                else
                {
                    text = MISSCC;
                    xc = 64 - 12 * 2;
                }
                int i = 0;
                for (i = 0; text[i] != '\0'; i++)
                {
                    drawChar(xc, 64, text[i], WHITE, RED, 2);
                    xc += 12;
                }
            }
        }
        TimerDisable(TIMERA0_BASE, TIMER_A);
        fillScreen(BLACK);
        message = ("Calculating score...");
        DisplayMessageC(message, 20);

        int misses = 0;
        int hits = 0;
        int i = 0;
        for (i = 0; results[i].lane != -2; i++)
        {
            if (results[i].lane == 1)
            {
                hits++;
            }
            else
            {
                misses++;
            }
        }
        Report("Hits: %i \tMisses: %i\n", hits, misses);

        // Buffer to hold the formatted JSON string
        char jsonData[128];

        snprintf(jsonData, sizeof(jsonData),
                 "{\"user\": \"%s\", \"miss\": %d, \"hit\": %d}", user, misses, hits);

        // Send JSON via HTTP POST
        http_postmsg(iSockID, jsonData);

        // Loop back to MENU state
    }
    //
    // Loop forever while the timers run.
    //
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
