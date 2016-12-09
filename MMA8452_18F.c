/* MMA8452_18F.c - I2C communication routines for accelerometer
 * Adapted for 18F series PICs
 *
 * 12-9-16: Adapted for Magic Wand using Linx and Manchester coding
 * 
 */
#include "MMA8452_18F.h"
#include <xc.h>

#define false 0
#define true !false
#define TRUE true
#define FALSE false

// #define ORIENTATION_DETECTION
#define MOTION_DETECTION

// This routine initiates I2C reads and writes
// It sends a START, then the DEVICE address,
// then the first REGISTER to read or write.
unsigned char sendREADcommand(unsigned char deviceID) {

    i2c_Start();    

    // Write device ID and WRITE command   
    i2c_SendControlByte(deviceID << 1, I2C_READ_BIT);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);

    return (TRUE);

}

unsigned char setRegister(unsigned char deviceID, unsigned char deviceREGISTER) {    

    i2c_Start();    

    // Write device ID and WRITE command   
    i2c_SendControlByte(deviceID << 1, I2C_WRITE_BIT);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);

    // Write first register to be addressed
    i2c_SendByte(deviceREGISTER);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);

    return (TRUE);

}

unsigned char writeByteToRegister(unsigned char deviceID, unsigned char deviceREGISTER, unsigned char dataByte) {

    i2c_Start();    

    // Write device ID    
    i2c_SendControlByte(deviceID << 1, I2C_WRITE_BIT);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);

    // Write first register to be addressed    
    i2c_SendByte(deviceREGISTER);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);        

    // Write data byte    
    i2c_SendByte(dataByte);

    // Verify that the byte was acknowledged    
    if (i2c_GetAcknowledge() == ACK_ERROR) return (FALSE);            
    
    // End of I2C transaction
    i2c_Stop();

    return (TRUE);

}

unsigned char readRegisters(unsigned char deviceID, unsigned char deviceREGISTER, unsigned char numRegisters, unsigned char *registerPtr) {
    unsigned char i;

    if (numRegisters >= MAXBUFFER)return (0);
    // if (!registerPtr)return (0);

    if (!setRegister(deviceID, deviceREGISTER)) return (FALSE);
    if (!sendREADcommand(deviceID)) return (FALSE);

    for (i = 0; i < numRegisters; i++) {            
            registerPtr[i] = i2c_ReadByte(); // Read a byte from slave
            if (i == numRegisters-1) i2c_SendAcknowledge(I2C_LAST);     // Leave acknowledge high for last byte
            else i2c_SendAcknowledge(I2C_MORE);                         // otherwise ACK all bytes except last byte
    }

    i2c_Stop();

    return (i);
}

#ifdef MOTION_DETECTION
unsigned char initMMA8452(void) {
    unsigned char accelData[4] = {0, 0, 0, 0};
    unsigned char commandByte;

    //if (!readRegisters(ACCELEROMETER_ID, WHO_AM_I, 1, accelData)) return (FALSE); // Read WHO_AM_I register
    //if (accelData[0] != 0x2A) return (FALSE); // WHO_AM_I should always be 0x2A

    if (!readRegisters(ACCELEROMETER_ID, CTRL_REG1, 1, accelData)) return (FALSE); 
    commandByte = accelData[0];
    commandByte &= 0xFE; // Set last bit to 0 for STANDBY mode
    commandByte &= 0xC7; // Clear the sample rate bits
    commandByte |= 0x18; //Set the sample rate bits to 400 Hz = 0x08, 100 Hz = 0x18, 200 Hz = 0x10, 800 Hz = 0x00, 50 Hz = 0x20
    if (!writeByteToRegister(ACCELEROMETER_ID, CTRL_REG1, commandByte)) return (FALSE);
        
    if (!writeByteToRegister(ACCELEROMETER_ID, POWER_MODE, 0b00000000)) return (FALSE);  // Use normal power modes
    
       
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x15, 0xF8)) return (FALSE); // Register 0x15 Motion Config: Enable Latch, Motion, Z-axis,  X-axis, Y-axis
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x17, 0x18)) return (FALSE); // Register 0x17 Set Threshold for > 1.5g:  1.5g/0.063g = 23.8; Round up to 24
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x18, 0x04)) return (FALSE); // Register 0x18 Debounce Counter: 40 ms debounce timing. This was 0x0A for 100 ms debounce
    
    // Set up interrupts for orientation detection
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2D, 0x04)) return (FALSE);  // Interrupt enable register: use motion detection
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2E, 0x04)) return (FALSE);  // Interrupt configuration register:  use INT1 PIN
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2C, 0x0A)) return (FALSE);   //  Interrupt control: int pin is active high, motion wakeup enabled
    if (!readRegisters(ACCELEROMETER_ID, 0x0C, 1, accelData)) return (FALSE);  // Read Interrupt Source register to make sure it's cleared
    
    if (!writeByteToRegister(ACCELEROMETER_ID, XYZ_DATA_CFG, 0x00)) return (FALSE); // Set range to 2 G, disable high pass filtering
    

    if (!readRegisters(ACCELEROMETER_ID, CTRL_REG1, 1, accelData)) return (FALSE); // Put in ACTIVE mode
    commandByte = accelData[0];
    commandByte |= 0x01;
    if (!writeByteToRegister(ACCELEROMETER_ID, CTRL_REG1, commandByte)) return (FALSE);

    return (TRUE);
}
#endif

#ifdef ORIENTATION_DETECTION
unsigned char initMMA8452(void) {
    unsigned char accelData[4] = {0, 0, 0, 0};
    unsigned char commandByte;

    //if (!readRegisters(ACCELEROMETER_ID, WHO_AM_I, 1, accelData)) return (FALSE); // Read WHO_AM_I register
    //if (accelData[0] != 0x2A) return (FALSE); // WHO_AM_I should always be 0x2A

    if (!readRegisters(ACCELEROMETER_ID, CTRL_REG1, 1, accelData)) return (FALSE); 
    commandByte = accelData[0];
    commandByte &= 0xFE; // Set last bit to 0 for STANDBY mode
    commandByte &= 0xC7; // Clear the sample rate bits
    commandByte |= 0x18; //Set the sample rate bits to 400 Hz = 0x08, 100 Hz = 0x18, 200 Hz = 0x10, 800 Hz = 0x00, 50 Hz = 0x20
    if (!writeByteToRegister(ACCELEROMETER_ID, CTRL_REG1, commandByte)) return (FALSE);
        
    if (!writeByteToRegister(ACCELEROMETER_ID, POWER_MODE, 0b00000000)) return (FALSE);  // Use normal power modes
    
       
    if (!readRegisters(ACCELEROMETER_ID, 0x11, 1, accelData)) return (FALSE); // ORIENTATION SETUP REGISTER
    commandByte = accelData[0];
    commandByte |= 0x40; // Set the PL_EN bit in PL_COFIG Register to enable the orientation detection.    
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x11, commandByte)) return (FALSE);            
    
    // Set up interrupts for orientation detection
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2D, 0b00010000)) return (FALSE);  // Interrupt enable register: use orientation interrupts
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2E, 0b00010000)) return (FALSE);  // Interrupt configuration register:  use INT1 PIN
    if (!writeByteToRegister(ACCELEROMETER_ID, 0x2C, 0x22)) return (FALSE);         //  Interrupt control: int pin is active high, orientation wakeup enabled
    if (!readRegisters(ACCELEROMETER_ID, INTERRUPT_SOURCE, 1, accelData)) return (FALSE);  // Do a dummy read to make sure register is cleared
    if (!writeByteToRegister(ACCELEROMETER_ID, PL_DEBOUNCE_COUNTER, 0x01)) return (FALSE); // This sets the debounce counter to 100 ms at 50 Hz was 0x05
    
    if (!writeByteToRegister(ACCELEROMETER_ID, XYZ_DATA_CFG, 0x00)) return (FALSE); // Set range to 2 G, disable high pass filtering
    

    if (!readRegisters(ACCELEROMETER_ID, CTRL_REG1, 1, accelData)) return (FALSE); // Put in ACTIVE mode
    commandByte = accelData[0];
    commandByte |= 0x01;
    if (!writeByteToRegister(ACCELEROMETER_ID, CTRL_REG1, commandByte)) return (FALSE);

    return (TRUE);
}
#endif



short convertValue(unsigned char MSBbyte, unsigned char LSBbyte) {
    short value;

    // Combine high and low bytes
    value = MSBbyte;
    value = value << 8;
    value = value | LSBbyte;

    //The registers are left align, here we right align the 12-bit integer
    value = value >> 4;

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if ((MSBbyte & 0x80) != 0) {
        value = ~value + 1;
        value = value * -1; // Transform into negative 2's complement #
    }
    return (value);
}


//	This routine sets up I2C functions for master mode

void initialize_I2C(void){
    SSPCON1 = 0x00;
    SSPCON1bits.SSPEN = 1;
    SSPCON1bits.SSPM3 = 1; // 1000 = I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))(4)
    SSPCON1bits.SSPM2 = 0;
    SSPCON1bits.SSPM1 = 0;
    SSPCON1bits.SSPM0 = 0;
    SSPCON1bits.CKP = 1;
    SSPADD = 9;	// FBUS = OSC/4 /(N+1)  OSC = 4, OCS =0.4; 10 IS JUST RIGHT
	SSPSTAT = 0;	// Clear MSSP status register

	// Make sure I2C is STOPPED:
	// i2c_Stop();
}


//  Send stop condition
// 	  - data low-high while clock high

void
i2c_Stop(void){
	SSPCON2bits.PEN = 1;  //  PEN = 1;			// Send STOP condition
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG
}

//	Send start condition

void
i2c_Start(void){
	SSPCON2bits.SEN = 1;			// Send START condition
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG
}


//	Send restart condition
/*
void
i2c_Restart(void){
	SSPCON2bits.RSEN = 1;			// Send REPEATED START condition
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG
}
*/

//	Send a byte to the slave
// 	  - returns true on error

unsigned char
i2c_SendByte(unsigned char byte){

	SSPBUF = byte;		// Send byte
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG

	return FALSE;
}

//	send control byte and data direction to the slave
//  	- 7-bit control byte (lsb ignored)
// 	  	- direction (0 = write )

unsigned char i2c_SendControlByte(unsigned char controlByte, unsigned char direction){
unsigned char outByte;

		outByte = controlByte | direction;
        return i2c_SendByte (outByte);
}

//	Check for an acknowledge from slave EEPROM after sending it a command or data
// 	  - returns ack or ~ack
unsigned char i2c_GetAcknowledge(void){
unsigned char ack;

	ack = SSPCON2bits.ACKSTAT;
    // if (ack != 0) printf ("\rACK ERROR");
	return ack;
}



unsigned char i2c_ReadByte(void){
unsigned char byte;
	
	SSPCON2bits.RCEN = 1;	// Enable receive mode for I2C
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG
	byte = SSPBUF;
	return (int)byte;
}

//	Send an (~)acknowledge to the slave
//		- status of I2C_LAST implies this is the last byte to be sent
//    	- if there are more bytes, then ACK must be brought low
//		- if there are no more bytes, then ACK is left high
//
// Returns nothing

void i2c_SendAcknowledge(unsigned char status){
        if ( status & 0x01) {
 			SSPCON2bits.ACKDT = 0; // drive line low -> more to come
        }
        else { 
 			SSPCON2bits.ACKDT = 1; // line left high -> last expected byte has been received
	}	
	SSPCON2bits.ACKEN = 1;	// Initiate acknowledge sequence
	while (PIR1bits.SSPIF == 0); // WAIT TILL SSP1IF SET IN PIR REGISTER
	PIR1bits.SSPIF = 0;			// CLEAR FLAG
}

