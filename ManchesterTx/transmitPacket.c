// #include <p18f2520.h>
#include <xc.h>
#include "delay.h"

#define TX_OUT 		LATAbits.LATA0
#define TRIG_OUT    PORTBbits.RB5

#define SHORT_LOW_LOOPS 200
#define SHORT_HIGH_LOOPS 80

#define LONG_LOW_LOOPS 500 // 255 
#define LONG_HIGH_LOOPS 400 // 233 //67

void xmitPacket(unsigned short numBytes, unsigned char *ptrDelay){
    // unsigned char delayTime, delayLoop;
    unsigned short delayTime, delayLoop;
    unsigned short index;

    TX_OUT = 1;
    delayLoop = SHORT_HIGH_LOOPS;    
    while (delayLoop) delayLoop--;
    TX_OUT = 0;    
    
    index = 0;
    while (index < numBytes) {
        delayTime = ptrDelay[index];

        if (delayTime == SHORT_PULSE_HI) {
            TX_OUT = 1;
            delayLoop = SHORT_HIGH_LOOPS;
        } else if (delayTime == LONG_PULSE_HI) {
            TX_OUT = 1;
            delayLoop = LONG_HIGH_LOOPS;
        } else if (delayTime == SHORT_PULSE_LOW) {
            TX_OUT = 0;
            delayLoop = SHORT_LOW_LOOPS;
        } else {
            TX_OUT = 0;
            delayLoop = LONG_LOW_LOOPS;
        }
        while (delayLoop) delayLoop--;
        index++;
    }
    
    if (TX_OUT) TX_OUT = 0;
    else TX_OUT = 1;    
    
    delayLoop = 0xFF;
    while (delayLoop) delayLoop--;
    
    if (TX_OUT) TX_OUT = 0;
    else TX_OUT = 1;    
    
    delayLoop = 0xFF;
    while (delayLoop) delayLoop--;

    TX_OUT = 0;
}

void xmitBreak(void) {
    unsigned short delayTime, delayLoop, numStartPulses;

    numStartPulses = 8;
    // Send sixteen fast pulses to settle RF

    do {
        TX_OUT = 1;
        delayLoop = 200;
        while (delayLoop) delayLoop--;

        TX_OUT = 0;
        delayLoop = 400;
        while (delayLoop) delayLoop--;

        numStartPulses--;
    } while (numStartPulses);
    
    // Send a 3 millisecond high pulse    
    TX_OUT = 1;
    delayTime = 40;
    while (delayTime) {
        delayLoop = 74;
        while (delayLoop)delayLoop--;
        delayTime--;
    }

    // Send a millisecond low pulse
    TX_OUT = 0;
    delayTime = 13;
    while (delayTime) {
        delayLoop = 60;
        while (delayLoop)delayLoop--;
        delayTime--;
    }
    
    // Send a half millisecond high pulse
    TX_OUT = 1;
    delayTime = 5;
    while (delayTime) {
        delayLoop = 74;
        while (delayLoop)delayLoop--;
        delayTime--;
    }

    // Send a half millisecond low pulse
    TX_OUT = 0;
    delayTime = 18;
    while (delayTime) {
        delayLoop = 20;
        while (delayLoop)delayLoop--;
        delayTime--;
    }
}

