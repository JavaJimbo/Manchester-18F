// #include <p18f2520.h>
#include <xc.h>

#define TX_OUT 		LATAbits.LATA0
#define NUMDELAYLOOPS 100

// unsigned char xmitPacket(unsigned char numBytes, unsigned char *arrDelay){
extern unsigned char xmitPacket(unsigned char numBytes, unsigned char *ptrDelay){
unsigned char delayTime, delayLoop, index;
 
    index = 0;
    while (index < numBytes){
        // Toggle output pin
        if (TX_OUT) TX_OUT = 0;
        else TX_OUT = 1;
         // Get next delay period
        delayTime = ptrDelay[index];

        while (delayTime) {
        // Create adequate delay
        delayLoop = NUMDELAYLOOPS;
        // Keep looping until delay is done
            while (delayLoop){
                Nop();
                Nop();                                
                delayLoop--;
            }
            delayTime--;
        }
        // Go to next element in delay array
        index++;        
    } 
    // Quit when there is no more data to send
    return(23);
}

