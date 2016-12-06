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
                              
    ;TransmitPacket.c: 9: unsigned char delayTime, delayLoop, index;
    ;TransmitPacket.c: 11: index = 0;
    movlw	0
    movwf	xmitPacket@index,c
                              
    ;TransmitPacket.c: 12: while (index < numBytes){
    goto	l825
l79:
                              
    ;TransmitPacket.c: 14: if (LATAbits.LATA0) LATAbits.LATA0 = 0;
    btfss	3977,0,c	;volatile
    goto	l80
    bcf	3977,0,c	;volatile
    goto	l811
l80:                         
    ;TransmitPacket.c: 15: else LATAbits.LATA0 = 1;
    bsf	3977,0,c	;volatile
l811:
    ;TransmitPacket.c: 17: delayTime = ptrDelay[index];
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
                      
    ;TransmitPacket.c: 19: while (delayTime) {
    goto	l821
l813:                      
    ;TransmitPacket.c: 21: delayLoop = 100;
    movlw	NUMDELAYLOOPS
    movwf	xmitPacket@delayLoop,c
                         
    ;TransmitPacket.c: 23: while (delayLoop){
    goto	l817
l85:
    ;TransmitPacket.c: 24: __nop();
    nop	
    ;TransmitPacket.c: 25: __nop();
    nop	

    ;TransmitPacket.c: 26: delayLoop--;
    decf	xmitPacket@delayLoop,f,c
l817:
    movf	xmitPacket@delayLoop,w,c
    btfss	status,2,c
    goto	l85
                         
    ;TransmitPacket.c: 27: }
    ;TransmitPacket.c: 28: delayTime--;
    decf	xmitPacket@delayTime,f,c
l821:
    movf	xmitPacket@delayTime,w,c
    btfss	status,2,c
    goto	l813
                          
    ;TransmitPacket.c: 29: }
    ;TransmitPacket.c: 31: index++;
    incf	xmitPacket@index,f,c
l825:
    movf	xmitPacket@numBytes,w,c
    subwf	xmitPacket@index,w,c
    btfss	status,0,c
    goto	l79
                         
    ;TransmitPacket.c: 32: }
    ;TransmitPacket.c: 34: return(23);
    movlw	23
    return	
    
    xmitPacket@numBytes equ 0x0011       
    xmitPacket@ptrDelay equ 0x000F  
    xmitPacket@delayLoop equ 0x0013      
    xmitPacket@delayTime equ 0x0012    
    xmitPacket@index equ 0x0014 
 
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
    NUMDELAYLOOPS equ	62
 
    END
