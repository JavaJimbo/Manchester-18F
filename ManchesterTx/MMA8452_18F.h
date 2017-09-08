/* 
 * File:   MMA8452 18F.h
 * Author: Jim
 *
 * Created 7-23-2015
 * Adapted from EEPROM routines 
 */

#ifndef MMA8452_H
#define	MMA8452_H

#define MAXBUFFER 64
#define MAX_I2C_REGISTERS 6

//Define a few of the registers that we will be accessing on the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I   0x0D
#define CTRL_REG1  0x2A
#define POWER_MODE 0x2B
#define ACCELEROMETER_ID 0x1D
#define ORIENTATION_STATUS 0x10
#define ORIENTATION_SETUP 0x11
#define INTERRUPT_CONTROL 0x2C
#define INTERRUPT_ENABLE_REG 0x2D
#define INTERRUPT_PIN_SELECT 0x2E
#define INTERRUPT_SOURCE 0x0C
#define PL_DEBOUNCE_COUNTER 0x12

#define ORIENTATION_MASK 0b00010000

#define GSCALE 2 // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.

#define EEPROM_ADDR 0xA0 			// This addresses the external EEPROM that holds the PIC program code

#define I2C_READ_BIT 0x01		/* read bit used with address */
#define I2C_WRITE_BIT 0x00		/* write bit used with address */

// #define I2C_ERROR	(-1)
#define I2C_LAST	FALSE		/* SendAck: no more bytes to send */
#define I2C_MORE	TRUE		/* SendAck: more bytes to send */

#define ACK_ERROR 1

// BUFFERSIZE is equal to the number of bytes in the internal program Write Buffer of the PIC 18F2550.
#define BUFFERSIZE	64				

extern unsigned char    i2c_ReadByte(void);
extern unsigned char 	i2c_GetAcknowledge(void);
extern unsigned char	i2c_SendControlByte(unsigned char, unsigned char);
extern unsigned char	i2c_SendByte(unsigned char);
extern void				i2c_Start(void);
extern void				i2c_Restart(void);
extern void				i2c_Stop(void);
extern void				i2c_SendAcknowledge(unsigned char);
extern unsigned char	i2c_read(unsigned char);
extern void 			initialize_I2C (void);

extern unsigned char TransmitOneByte(unsigned char data);
extern unsigned char initMMA8452(void);
extern unsigned char setRegister(unsigned char deviceID, unsigned char deviceREGISTER);
extern unsigned char sendREADcommand(unsigned char deviceID);
extern unsigned char readRegisters(unsigned char deviceID, unsigned char deviceREGISTER, unsigned char numRegisters, unsigned char *registerPtr);
extern unsigned char writeByteToRegister(unsigned char deviceID, unsigned char deviceREGISTER, unsigned char dataByte);
short getTwosComplement(unsigned char MSBbyte, unsigned char LSBbyte);
unsigned char initMMA8452(void);
unsigned char resetMMA8452(void);

#endif	/* MMA8452_H */

