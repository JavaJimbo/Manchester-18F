/*  LINX Transmit - MPLABX project
 *  For Magic Wand boards
 *  C language written for for 18F2520 on XC8 Compiler
 *
 *  5-24-14 
 *  11-19-16
 *  11-20-16: Works with sleep mode. Sends ">ABCDEF<\r" at 9600 baud on Tx UART pin.
 *  Uses RA0 pin to send start bits.
 *  11-25-16: Got Manchester Coding working on magic wands 
 *  with 18LF2520 running at 16 Mhz with Timer 2 rolling over at 100 uS 
 *  11-27-16: Simplified Manchester transmitting / receiving and greatly improved reliability.
 *  Transmit errors rarely occur. Distance improved significantly.
 *  Created xmitTest.c file.
 *  Added multibyte packets and CRC check. Works great! 
 *  12-6-16: Works sending Manchester bytes, no assembly code.
 *  12-7-16: Added additional optimization, can handle up to 64 bytes: length + data + CRC
 *  12-8-16: Set NUM_DATA_BYTES to maximum of 61 data bytes,
 *  12-9-16: Hooked up MMA8452Q accelerometer to wake up from Sleep on orientation change.
 *           Added wake up an acceleration change - works great waving wand around.
 *  12-12-16: Enabled SLEEP mode on MMA8452Q. Seems to draw about 120 uA when sleeping.
 *          Sampling rate in WAKE mode set to 100 Hz, SLEEP mode is 12.5 Hz.
 *          Timeout in WAKE mode before going back to SLEEP is about 21 seconds.
 */

#include <xc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "delay.h"
#include "MMA8452_18F.h"
#pragma config IESO = OFF, OSC = HS, FCMEN = OFF, BOREN = OFF, PWRT = ON, WDT = OFF, CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON, DEBUG = OFF, STVREN = OFF, XINST = OFF, LVP = OFF, CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF  // For 18F2520

#define false 0
#define true !false
#define TRUE true
#define FALSE false

#define PDownOut 	PORTAbits.RA1
#define TX_OUT 		LATAbits.LATA0
#define PUSHin 		PORTBbits.RB0
#define TRIG_OUT    PORTBbits.RB5

// #define ORIENTATION_DETECTION
#define MOTION_DETECTION

#define ORIENTATION_BIT 0x10
#define MOTION_BIT 0x04
#define INTERRUPT_STATUS ORIENTATION_BIT

void init(void);

extern void xmitPacket(unsigned short numBytes, unsigned char *ptrDelay);
extern void xmitBreak(void);

#define MAXPACKETBYTES 1024
unsigned char arrDataPacket[MAXPACKETBYTES];
#define MAX_DATA_BYTES 64
unsigned char commandBuffer[MAX_DATA_BYTES];   
unsigned short createDataPacket(unsigned char *ptrData, unsigned short numDataBytes, unsigned char *ptrPacket);

#define HIGH_STATE 1
#define LOW_STATE 0

extern unsigned short CRCcalculate (unsigned char *message, unsigned char nBytes);

#define MAX_I2C_REGISTERS 6
unsigned char PORTBreg;
unsigned char pushFlag = FALSE;

void main() {
unsigned short numBytesToSend;
unsigned char i, j;  
unsigned char command = 0;    
unsigned char accelerometerBuffer[MAX_I2C_REGISTERS];

#ifdef ORIENTATION_DETECTION
unsigned char orientationData = 0;
#endif

#ifdef MOTION_DETECTION
unsigned char motionDetection = 0;
unsigned char sysModRegister = 0;
#endif

unsigned char interruptSource = 0, intDataReg = 0;
short rawVectx, rawVecty, rawVectz;
unsigned char initResult = 0;

    
union {
    unsigned char byte[2];
    unsigned short integer;
} convert;    
    
    init();
    initialize_I2C();
    initResult = initMMA8452();
    
    while(1){                        
        
#ifdef ORIENTATION_DETECTION       
        do {
            readRegisters(ACCELEROMETER_ID, 0x0C, 1, &interruptSource);     // Read Interrupt Source register            
            if (interruptSource & 0x10)                                     // If this is an orientation interrupt,                  
                readRegisters(ACCELEROMETER_ID, 0x10, 1, &orientationData); // then read orientation status register
        } while (interruptSource != 0);
#endif
        
#ifdef MOTION_DETECTION
        do {
            readRegisters(ACCELEROMETER_ID, 0x0C, 1, &interruptSource);     // Read Interrupt Source register            
            if (interruptSource){                                           
                readRegisters(ACCELEROMETER_ID, 0x0B, 1, &sysModRegister);  // Clear Sys Mod Register
                readRegisters(ACCELEROMETER_ID, 0x16, 1, &motionDetection); // If this is an motion detection interrupt, then read motion register
            }
        } while (interruptSource != 0);
#endif        
        
        PDownOut = 0;
        TX_OUT = 0;        
        Sleep();    
        
        readRegisters(ACCELEROMETER_ID, 0x01, MAX_I2C_REGISTERS, accelerometerBuffer);
        rawVectx = convertValue(accelerometerBuffer[0], accelerometerBuffer[1]);
        rawVectz = convertValue(accelerometerBuffer[2], accelerometerBuffer[3]);
        rawVecty = convertValue(accelerometerBuffer[4], accelerometerBuffer[5]);              
        
#define NUM_DATA_BYTES 7
        i = 0; 
        commandBuffer[i++] = NUM_DATA_BYTES;                 

#ifdef ORIENTATION_DETECTION         
        commandBuffer[i++] = orientationData;
#endif        
        
#ifdef MOTION_DETECTION         
        commandBuffer[i++] = motionDetection;
        motionDetection = 0x00;
#endif                
        convert.integer = rawVectx;
        commandBuffer[i++] = convert.byte[0];
        commandBuffer[i++] = convert.byte[1];        
        
        convert.integer = rawVectz;
        commandBuffer[i++] = convert.byte[0];
        commandBuffer[i++] = convert.byte[1];        
        
        convert.integer = rawVecty;
        commandBuffer[i++] = convert.byte[0];
        commandBuffer[i++] = convert.byte[1];                

        convert.integer = CRCcalculate(&commandBuffer[1], NUM_DATA_BYTES);
        commandBuffer[i++] = convert.byte[0];
        commandBuffer[i++] = convert.byte[1];
        
        numBytesToSend = createDataPacket(commandBuffer, i, arrDataPacket);       
        
        //xmitStartSequence();
        //xmitData(commandBuffer, 8);
        //xmitStopSequence();
        //TRIG_OUT = 0;

        TRIG_OUT = 1;
        PDownOut = 1;
        xmitBreak();
        xmitPacket(numBytesToSend, arrDataPacket);
        PDownOut = 0;
        TX_OUT = 0;
        TRIG_OUT = 0;
        
        if (pushFlag){
            do {
                DelayMs(20);
            } while (!PUSHin);
            DelayMs(20);
            while (!PUSHin);        
            pushFlag = FALSE;
        }
    }
}

void init(void) {
    INTCONbits.GIE = 0; // Global interrupt disabled

    ADCON0 = 0;
    ADCON1 = 0b00001111; // PORT A is all digital

    TRISB = 0b11011111; // RB5 is LED out
    INTCON2bits.RBPU = 0; // Enable pullups
    TRISC = 0b11111111;
    // TRISC = 0b10111111; // RC6 is TX output

    TXSTAbits.TXEN = 0; // Disable UART Tx        
    TRISC = 0b11111111; // Set UART Tx pin to input
    TRISA = 0b11111100; // Set RA0 to output
    // TRISA = 0b11111101; // 

/*    
    // Set up the UART         
    TXSTAbits.BRGH = 0; // Low speed baud rate
    SPBRG = 51; // Baudrate = 4800 @ 16 Mhz clock
    TXSTAbits.SYNC = 0; // asynchronous
    RCSTAbits.SPEN = 1; // enable serial port pins
    RCSTAbits.CREN = 1; // enable reception
    RCSTAbits.SREN = 0; // no effect
    PIE1bits.TXIE = 0; // disable tx interrupts 
    PIE1bits.RCIE = 0; // disable rx interrupts 
    TXSTAbits.TX9 = 0; // 8-bit transmission
    RCSTAbits.RX9 = 0; // 8-bit reception
    TXSTAbits.TXEN = 0; // Enable transmitter
    BAUDCONbits.TXCKP = 0; // Do not invert transmit and receive data
    BAUDCONbits.RXDTP = 0;

    // Set up Timer 2 to roll over every 100 uS:	
    T2CON = 0x00;
    T2CONbits.T2CKPS1 = 0; // 1:4 prescaler  was 1:1 for 10uS rolloever
    T2CONbits.T2CKPS0 = 1;
    T2CONbits.T2OUTPS3 = 0; // 1:1 postscaler
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS0 = 0;
    PR2 = 10; // Was 40 for 10 uS rollover
    T2CONbits.TMR2ON = 1; // Timer 2 ON
*/ 

    // Set up interrupts. 
    INTCON = 0x00;          // Clear all interrupts
    INTCONbits.INT0IE = 1;  // Enable pushbutton interrupts
    INTCONbits.RBIE = 1;    // Enable accelerometer interrupts

    INTCON2 = 0x00;
    INTCON2bits.RBPU = 0;       // Enable Port B pullups 
    INTCON2bits.INTEDG0 = 0;    // Interrupt on falling edge of RB0 pushbutton wakes up PIC from sleep mode.
    INTCONbits.GIE = 1;         // Enable global interrupts           
}



static void interrupt isr(void) {
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0;
        pushFlag = TRUE;
    }
    
    if (INTCONbits.RBIF) {
        PORTBreg = PORTB;
        INTCONbits.RBIF = 0;
    }
}


unsigned short createDataPacket(unsigned char *ptrData, unsigned short numDataBytes, unsigned char *ptrPacket){
unsigned char byteMask, dataBit, previousDataBit, i, state;
unsigned short dataIndex, packetIndex;

    if ( (numDataBytes > MAX_DATA_BYTES) || (ptrData == NULL) || (ptrPacket == NULL) )
        return(0);
    
    previousDataBit = 0;
    state = LOW_STATE;
    
    packetIndex = dataIndex = 0;
    byteMask = 0x01;
    
    while (dataIndex < numDataBytes){
        byteMask = 0x01;
        for (i = 0; i < 8; i++) {
            if (byteMask & ptrData[dataIndex]) dataBit = 1;
            else dataBit = 0;

            // If data bit is same state as previous data bit,
            // then packet gets two short pulses:
            if (dataBit == previousDataBit){    
                if (state == HIGH_STATE){
                    ptrPacket[packetIndex++] = SHORT_PULSE_HI;
                    ptrPacket[packetIndex++] = SHORT_PULSE_LOW;
                }
                else{
                    ptrPacket[packetIndex++] = SHORT_PULSE_LOW;
                    ptrPacket[packetIndex++] = SHORT_PULSE_HI;                    
                }
            }
            // Otherwise, if data bit is different than previous,
            // then packet gets one long pulse:
            else {
                if (state == HIGH_STATE){
                    state = LOW_STATE;
                    ptrPacket[packetIndex++] = LONG_PULSE_HI;
                }
                else {
                    state = HIGH_STATE;
                    ptrPacket[packetIndex++] = LONG_PULSE_LOW;
                }
            }

            previousDataBit = dataBit;
            byteMask = byteMask << 1;
        }
        dataIndex++;
    }
    if (packetIndex >= MAXPACKETBYTES) return (0);
    else return(packetIndex);
}

/*
void putch(unsigned char TxByte) {
    while (!PIR1bits.TXIF); // set when register is empty 
    TXREG = TxByte;
    return;
}
*/

// if (!writeByteToRegister(ACCELEROMETER_ID, 0x16, )) return (FALSE); // Register 0x16 Motion/Freefall Source Detection                