#include <xc.inc>
GLOBAL _xmitPacket 
SIGNAT _xmitPacket,4217 
PSECT mytext,local,class=CODE,reloc=2

    opt stack 0
    _xmitPacket:
    opt stack 29
                          
                          ;incstack = 0
                           ;xmitPacket@numBytes stored from wreg
               	movwf	xmitPacket@numBytes,c
                         
                          ;transmitPacket.c: 10: unsigned char delayTime, delayLoop, index;
                            ;transmitPacket.c: 12: index = 0;
               	movlw	0
                	movwf	xmitPacket@index,c
                        
                           ;transmitPacket.c: 13: while (index < numBytes){
              	goto	l1042
                     l108:
                          
                           ;transmitPacket.c: 15: if (LATAbits.LATA0) LATAbits.LATA0 = 0;
               	btfss	3977,0,c	;volatile
                 	goto	l109
               	bcf	3977,0,c	;volatile
               	goto	l1026
                    l109:
                          
                          ;transmitPacket.c: 16: else LATAbits.LATA0 = 1;
               	bsf	3977,0,c	;volatile
                     l1026:
                            
                          ;transmitPacket.c: 18: delayTime = ptrDelay[index];
                  	movf	xmitPacket@index,w,c
                 	mullw	1
                	movf	prodl,w,c
                	addwf	xmitPacket@ptrDelay,w,c
              	movwf	fsr2l,c
                 	movf	prodh,w,c
               	addwfc	xmitPacket@ptrDelay+1,w,c
                	movwf	fsr2h,c
                	movf	indf2,w,c
              	movwf	xmitPacket@delayTime,c
                          
                         ;transmitPacket.c: 20: while (delayTime) {
              	goto	l1038
                   l112:
                          
                         ;transmitPacket.c: 22: if (LATAbits.LATA0) delayLoop = 55;
             	btfss	3977,0,c	;volatile
                	goto	l1030
               	movlw	NUM_HIGH_LOOPS
               	goto	L1
                     l1030:
                           
                           ;transmitPacket.c: 23: else delayLoop = 85;
                	movlw	NUM_LOW_LOOPS
                    L1:
             	movwf	xmitPacket@delayLoop,c
                          
                       ;transmitPacket.c: 25: while (delayLoop) delayLoop--;
                	goto	l1034
                     l1032:
                	decf	xmitPacket@delayLoop,f,c
                       l1034:
                 	movf	xmitPacket@delayLoop,w,c
                  	btfss	status,2,c
                 	goto	l1032
                        
                           ;transmitPacket.c: 26: delayTime--;
                 	decf	xmitPacket@delayTime,f,c
                       l1038:
              	movf	xmitPacket@delayTime,w,c
              	btfss	status,2,c
              	goto	l112
                          
                           ;transmitPacket.c: 27: }
                           ;transmitPacket.c: 29: index++;
                	incf	xmitPacket@index,f,c
                     l1042:
                 	movf	xmitPacket@numBytes,w,c
                 	subwf	xmitPacket@index,w,c
                	btfsc	status,0,c
                  	return	
               	goto	l108
                    ; __end_of_xmitPacket:
 
	xmitPacket@numBytes	equ 0x0011
	xmitPacket@ptrDelay	equ 0x000F
	xmitPacket@delayLoop	equ 0x0013 
	xmitPacket@delayTime	equ 0x0012
	xmitPacket@index	equ 0x0014
 
    tblptru	equ	0xFF8
    tblptrh	equ	0xFF7
    tblptrl	equ	0xFF6
    tablat	equ	0xFF5
    prodh	equ	0xFF4
    prodl	equ	0xFF3
    postinc0	equ	0xFEE
    postdec1	equ	0xFE5
    fsr1l	equ	0xFE1
    indf2	equ	0xFDF
    fsr2h	equ	0xFDA
    fsr2l	equ	0xFD9
    status	equ	0xFD8	
    NUM_LOW_LOOPS equ 200
    NUM_HIGH_LOOPS equ 25
 
    END
