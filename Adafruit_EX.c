#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "oled_test.h"
//#define pgm_read_byte(addr) (*(const unsigned char *)(addr))


void drawArrow(){

}
// return 1 if reached bottom
int drawDown(int x0, int* y0a, unsigned int color){
    if(*y0a < 0){
            return;
    }
    if(*y0a >= 98){
        *y0a = -120;
        return;
    }
    int y0 = *y0a;
    drawLine(x0 + 15, y0 + 30, x0, y0 + 15, color);
    drawLine(x0 + 6, y0 + 9, x0 + 10, y0 + 13, color);
    drawLine(x0+10, y0 + 4, x0+14, y0, color);
    drawFastVLine(x0+10, y0+4, 9, color);
    drawCircleHelper(x0+6,y0+15, 6, 0x01, color);

    drawLine(x0 + 15, y0 + 30, x0 + 30, y0 + 15, color);
    drawLine(x0 + 24, y0 + 9, x0 + 20, y0 + 13, color);
    drawLine(x0+20, y0 + 4, x0+16, y0, color);
    drawFastVLine(x0+20, y0+4, 9, color);
    drawCircleHelper(x0+ 24,y0+15, 6, 0x02, color);

}

void drawUp(int x0, int* y0a, unsigned int color){
    if(*y0a < 0){
            return;
    }
    if(*y0a >= 98){
        *y0a = -120;
        return;
    }
    int y0 = *y0a;
    drawLine(x0 + 15, y0, x0, y0 + 15, color);
    drawLine(x0 + 6, y0 + 21, x0 + 10, y0 + 17, color);
    drawLine(x0+10, y0 + 26, x0+14, y0 + 30, color);
    drawFastVLine(x0+10, y0+17, 9, color);
    drawCircleHelper(x0+6,y0+15, 6, 0x08, color);

    drawLine(x0 + 15, y0, x0 + 30, y0 + 15, color);
    drawLine(x0 + 24, y0 + 21, x0 + 20, y0 + 17, color);
    drawLine(x0+20, y0 + 26, x0+16, y0 + 30, color);
    drawFastVLine(x0+20, y0+17, 9, color);
    drawCircleHelper(x0+ 24,y0+15, 6, 0x04, color);

}


void drawLeft(int x0, int* y0a, unsigned int color){
    if(*y0a < 0){
            return;
    }
    if(*y0a >= 98){
        *y0a = -120;
        return;
    }
    int y0 = *y0a;

    drawLine(x0, y0 + 15, x0 + 15, y0, color);
    drawLine(x0 + 21, y0 + 6, x0 + 17, y0 + 10, color);
    drawLine(x0 + 26, y0 + 10, x0 + 30, y0 + 14, color);
    drawFastHLine(x0 + 17, y0 + 10, 9, color);
    drawCircleHelper(x0 + 15, y0 + 6, 6, 0x02, color);  // top-left corner

    drawLine(x0, y0 + 15, x0 + 15, y0 + 30, color);
    drawLine(x0 + 21, y0 + 24, x0 + 17, y0 + 20, color);
    drawLine(x0 + 26, y0 + 20, x0 + 30, y0 + 16, color);
    drawFastHLine(x0 + 17, y0 + 20, 9, color);
    drawCircleHelper(x0 + 15, y0 + 24, 6, 0x04, color);  // bottom-left corner
}



void drawRight(int x0, int* y0a, unsigned int color){
    if(*y0a < 0){
            return;
    }
    if(*y0a >= 98){
        *y0a = -120;
        return;
    }
    int y0 = *y0a;

    drawLine(x0 + 30, y0 + 15, x0 + 15, y0, color);
    drawLine(x0 + 9, y0 + 6, x0 + 13, y0 + 10, color);
    drawLine(x0 + 4, y0 + 10, x0, y0 + 14, color);
    drawFastHLine(x0 + 4, y0 + 10, 9, color);
    drawCircleHelper(x0 + 15, y0 + 6, 6, 0x01, color);  // top-right corner

    drawLine(x0 + 30, y0 + 15, x0 + 15, y0 + 30, color);
    drawLine(x0 + 9, y0 + 24, x0 + 13, y0 + 20, color);
    drawLine(x0 + 4, y0 + 20, x0, y0 + 16, color);
    drawFastHLine(x0 + 4, y0 + 20, 9, color);
    drawCircleHelper(x0 + 15, y0 + 24, 6, 0x08, color);  // bottom-right
}
