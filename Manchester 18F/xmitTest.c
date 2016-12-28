#include <xc.h>

void xmitData(unsigned char *ptrData, unsigned char numBytes);
void xmitStartSequence(void);
void xmitStopSequence(void);
#define TX_OUT LATAbits.LATA0

void xmitData(unsigned char *ptrData, unsigned char numBytes){
    unsigned char dataVal, i, j, k, mask;    
        
    for (i = 0; i < numBytes; i++){
        dataVal = ptrData[i];
        mask = 0x01;
        for (j = 0; j < 8; j++){
            if ((dataVal & mask) != 0){
                if (TX_OUT){k = 100; while(k--);}
                TX_OUT = 1;                
                k = 50; while(k--);
                TX_OUT = 0;
                k = 110; while(k--);
            }
            if ((dataVal & mask) == 0){
                if (!TX_OUT){k = 100; while(k--);}
                TX_OUT = 0;
                k = 110; while(k--);
                TX_OUT = 1;
                k = 50; while(k--);         
            }      
            mask = mask << 1;
        }  
    }    
}

void xmitStartSequence(void){
    unsigned short k;
    
    TX_OUT = 1;
    k = 66; while(k--);
    TX_OUT = 0;
    k = 66; while(k--);
    
    TX_OUT = 1;
    k = 66; while(k--);
    TX_OUT = 0;
    k = 66; while(k--);    
    
    TX_OUT = 1;
    k = 66; while(k--);
    TX_OUT = 0;
    k = 66; while(k--);
       
    TX_OUT = 1;
    k = 2000; while(k--);
    TX_OUT = 0;
    k = 400; while(k--);
    TX_OUT = 1;
    k = 400; while(k--);
    TX_OUT = 0;
    k = 120; while(k--);
    TX_OUT = 1;
    k = 60; while(k--);
}

void xmitStopSequence(void){
unsigned short k;    

    if (TX_OUT == 0){
        TX_OUT = 1;
        k = 50; while(k--);
        TX_OUT = 0;
    }
    else {
        TX_OUT = 0;
        k = 110; while(k--);
        TX_OUT = 1;   
        k = 50; while(k--);   
        TX_OUT = 0;
    }
}