//I2C.h
// Runs on TM4C123.
// Enables I2C1 to communicate with a device.
// Single Master (TM4C123).
// Kirabo Nsereko
// 1/10/2021


#include "tm4c123gh6pm.h"
#include <stdint.h>

#ifndef I2C_H
#define I2C_H

// Initiate I2C channel 1.
// Configuration:
// SCL clock speed of 100 Kbps
// PA6: SCL
// PA7: SDA
void I2C1_Init(void);

// Configures Master for Transmit or Recieve.
// Inputs: 1 (read) or 0 (write)
// Outputs: none
void RW(uint8_t bit);

// Set slave address.
// Inputs: slave address
// Outputs: none
void Set_Slave_Addr(uint8_t addr);

// Master sends a byte of data.
// Inputs: data to transmit, 
//         conditions: generate  master enable, start, and/or stop bit, etc.
// Outputs: error code. Look at TM4C123 data sheet for further explanation. 
uint8_t Send_Byte(uint8_t data, uint8_t conditions);

// Master receives a byte of data.
// Inputs: address of variable to save received data, 
//         conditions: generate  master enable, start, and/or stop bit, etc.
// Outputs: error code. Look at TM4C123 data sheet for further explanation.
uint8_t Read_Byte(uint8_t *data, uint8_t conditions);

#endif
