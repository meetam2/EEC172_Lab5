

#include <stdio.h>
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"

#include "gpio.h"
#include "i2c_if.h"
#include "interrupt.h"
#include "pin_mux_config.h"
#include "systick.h"
#include "timer.h"
#include "timer_if.h"
#include "uart_if.h"
#include "input_if.h"

#include "game.h"

const PinSetting CAP = { .port = GPIOA1_BASE, .pin = 0x20};
const PinSetting BUTTON = { .port = GPIOA1_BASE, .pin = 0x20};
const PinSetting switch2 = { .port = GPIOA2_BASE, .pin = 0x40};
const PinSetting switch3 = { .port = GPIOA1_BASE, .pin = 0x20};
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (1 != iRetVal) \
                                     return  iRetVal;}

extern Note notes[];
extern Note results[];
extern int noteIndex;
extern unsigned long hitThreshold;
extern int INVERTY;
extern int INVERTX;


//****************************************************************************
//// track systick counter periods elapsed
//// if it is not 0, we know the transmission ended
//volatile int systick_cnt = 0;
//
//// track if IR interrupt detected. Resets after transmission ended
//volatile int IRFlag = 0;
//
//// track if IR transmission finished
//volatile int IRFinish = 0;
//
//// track how many (falling) edges detected in one transmission
//volatile int edgeCount = 0;
//
//// Systick register value
//unsigned long reg = 0;
//
//// Buffer for pulse lengths
//unsigned long buffer[100];
//
//// Track the remote button pushed previously and currently
//int currentButton = -2;
//int prevButton = -1;
//int buttonIndex = 0; //index for set of letters the currentButton corresponds to (e.g. {A,B,C} for button 1)

//****************************************************************************

//****************************************************************************
//
//! Parses the readreg command parameters and invokes the I2C APIs
//!
//! \param ucDevAddr is the 7-bit I2C slave address
//! \param pucData is the pointer to the read data to be placed
//! \param ucRegOffset is the register address in the i2c device
//! \param ucLen is the length of data to be read
//!     To read the new data flag and the acceleration data for the x, y and z axes, use parameters (0x18, buffer, 0x2, 6)
//!     To read the acceleration data for the x, y and z axes, respectively, use parameters (0x18, buffer, 3, 1) (0x18, buffer, 5, 1) (0x18, buffer, 7, 1)
//!
//! This function
//!    1. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
readReg(unsigned char ucDevAddr, unsigned char *pucData, unsigned char ucRegOffset, unsigned char ucRdLen)
{
    //unsigned char aucRdDataBuf[256];
    //
    // Write the register address to be read from.
    // Stop bit implicitly assumed to be 0.
    //
    RET_IF_ERR(I2C_IF_Write(ucDevAddr, &ucRegOffset,1,0));

    //
    // Read the specified length of data
    //
    RET_IF_ERR(I2C_IF_Read(ucDevAddr, &pucData[0], ucRdLen));

    //  Uncomment to print read contents
//    int bufferIndex = 0;
//    printf("READ CONTENTS (0x): ");
//    while(bufferIndex < ucRdLen){
//        printf("%d, ", pucData[bufferIndex]);
//        bufferIndex++;
//    }
//    printf("\n");


    return 0;
}

//****************************************************************************
//
//! Reads BMA222 accelerometer for the acceleration data for the x, y and z axes, respectively
//!
//! \param pucData is the pointer to array[3] where data will be placed
//!         values range between -64, 64
//!
//! This function
//!    1. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int getAcc(int iData[3]){
    unsigned char buffer[6];

    //  Read 6 bytes from accelerometer
    RET_IF_ERR(readReg(0x18, &buffer[0], 2, 6));

    // Convert and assign to integer array. If INVERT is true, then the value will be 255 - value
    iData[0] = (INVERTX)? 255 - (int)buffer[1] : (int)buffer[1];
    iData[1] = (INVERTY)? 255 - (int)buffer[3] : (int)buffer[3];
    iData[2] = (int)buffer[5];

    //  Map the range from 0-255 to -64-64
    int i = 0;
    for(i=0; i<3; i++){
        if(iData[i] > 128){
            iData[i] = iData[i] - 255;
        }
        if(iData[i] < -64) {iData[i] = -64;}
        else if(iData[i] > 64) {iData[i] = 64;}
    }

    return 0;
}

/**
 * Initializes GPIO Interrupts
 */
void GPIOInit(void) {
////    GPIOIntRegister(switch2.port, SW2IntHandler);
//    GPIOIntRegister(switch2.port, GameTapIntHandler(switch2.port, switch2.pin));
//    GPIOIntTypeSet(switch2.port, switch2.pin, GPIO_RISING_EDGE);
////    GPIOIntTypeSet(IR.port, IR.pin, GPIO_FALLING_EDGE);
//    GPIOIntClear(switch2.port, switch2.pin);
//    GPIOIntEnable(switch2.port, switch2.pin);
    SetGameTapConfig(switch2.port, switch2.pin);
    GPIO_IF_ConfigureNIntEnable(switch2.port, switch2.pin, GPIO_RISING_EDGE, GameTapIntHandler);
}

void ConfigureNIntEnable(unsigned int uiGPIOPort,
                         unsigned char ucGPIOPin,
                         unsigned int uiIntType,
                         void (*pfnIntHandler)(void)){
//    GPIOIntRegister(uiGPIOPort, );
//    GPIOIntTypeSet(switch2.port, switch2.pin, GPIO_RISING_EDGE);
////    GPIOIntTypeSet(IR.port, IR.pin, GPIO_FALLING_EDGE);
//    GPIOIntClear(switch2.port, switch2.pin);
//    GPIOIntEnable(switch2.port, switch2.pin);
}


void IRIntHandler();
void ButtonIntHandler();
void SW2IntHandler(){
    unsigned long ulstatus = GPIOIntStatus(switch2.port, true);
    GPIOIntClear(switch2.port, ulstatus);

    // Disable interrupt
    if (ulstatus & switch2.pin){
//        MAP_TimerEnable(TIMERA0_BASE, TIMER_A);   // Start debounce timer
//                MAP_GPIOIntDisable(GPIOA1_BASE, GPIO_PIN_5);  // Disable button interrupt
        GPIOIntDisable(switch2.port, switch2.pin);
    }

    unsigned int time = TimerValueGet(TIMERA0_BASE, TIMER_A);   // Time until next note
    long time_diff = notes[noteIndex].delay - time;             // Time after prev note

    int hit = 0;
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
    Report("SW2 pressed. Timer value: %u \tDifference: %u \tHIT: %i\n", time, time_diff, hit);
    // Enable interrupt
    GPIOIntEnable(switch2.port, switch2.pin);
}

void SW3IntHandler(){
    unsigned long ulstatus = UARTIntStatus(switch3.port, true);
    UARTIntClear(switch3.port, ulstatus);
}

