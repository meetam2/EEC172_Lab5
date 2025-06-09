
#ifndef __INPUTIF_H__
#define __INPUTIF_H__


#define BTN_ONE    0b1010000000000010000000001000000000000100000001001UL
#define BTN_TWO    0b1010000000000010000000001000000001000100010001001UL
#define BTN_THREE  0b1010000000000010000000001000000000100100001001001UL
#define BTN_FOUR   0b1010000000000010000000001000000001100100011001001UL
#define BTN_FIVE   0b1010000000000010000000001000000000010100000101001UL
#define BTN_SIX    0b1010000000000010000000001000000001010100010101001UL
#define BTN_SEVEN  0b1010000000000010000000001000000000110100001101001UL
#define BTN_EIGHT  0b1010000000000010000000001000000001110100011101001UL
#define BTN_NINE   0b1010000000000010000000001000000000001100000011001UL
#define BTN_ZERO   0b1010000000000010000000001000000001001100010011001UL
#define BTN_ENTER  0b1010000000000010000000001000000000100101001001011UL
#define BTN_DELETE 0b1010000000000010000000001000000001000010010000101UL
//
//char letterSets[11][12] = {
//     "            ", "", "ABCABCABCABC", "DEFDEFDEFDEF", "GHIGHIGHIGHI", "JKLJKLJKLJKL", "MNOMNOMNOMNO", "PQRSPQRSPQRS", "TUVTUVTUVTUV", "WXYZWXYZWXYZ", ""
//    };
#define UART_BASE UARTA0_BASE
#define CONSOLE_PERIPH  PRCM_UARTA0
#define UART_BAUD_RATE  115200
#define SYSCLK          80000000

// some helpful macros for systick

// the cc3200's fixed clock frequency of 80 MHz
#define SYSCLKFREQ 80000000ULL

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

#define START_BIT_CLKS 3070000UL   //SYSTICK_RELOAD_VAL - Start bit pulse length in cycles

#define IR_LEN 49   // Minimum number of bits expected in each IR transmission


typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;


//const PinSetting IR = { .port = GPIOA1_BASE, .pin = 0x20};

void GPIOInit(void);
void IRIntHandler();
void ButtonIntHandler();
void SW2IntHandler();
void SW3IntHandler();

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
int readReg(unsigned char ucDevAddr, unsigned char *pucData, unsigned char ucRegOffset, unsigned char ucRdLen);


#endif  /*  __INTPUFIF_H__ */
