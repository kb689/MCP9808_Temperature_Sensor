// I2C.c
// Runs on TM4C123.
// Enables I2C1 to communicate with a device.
// Single Master (TM4C123).
// Kirabo Nsereko
// 1/10/2021

#include "I2C.h"


// Initiate I2C channel 1.
// Configuration:
// SCL clock speed of 100 Kbps
// PA6: SCL
// PA7: SDA
void I2C1_Init(void){
  //1. Enable the I2C clock using the RCGCI2C register in the System Control module (see page 348).
	SYSCTL_RCGCI2C_R |= (1<<1);
  //2. Enable the clock to the appropriate GPIO module via the RCGCGPIO register in the System
  //Control module (see page 340). To find out which GPIO port to enable, refer to Table
  //23-5 on page 1351.
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; // PortA
  //3. In the GPIO module, enable the appropriate pins for their alternate function using the
  //GPIOAFSEL register (see page 671). To determine which GPIOs to configure, see Table
  //23-4 on page 1344.
	GPIO_PORTA_AFSEL_R |= (1<<6)|(1<<7);
	GPIO_PORTA_DEN_R |= (1<<6)|(1<<7);
  //4. Enable the I2CSDA pin for open-drain operation. See page 676.
	GPIO_PORTA_ODR_R |= (1<<7);
  //5. Configure the PMCn fields in the GPIOPCTL register to assign the I2C signals to the appropriate
  //pins. See page 688 and Table 23-5 on page 1351.
	GPIO_PORTA_PCTL_R |= (3<<24)|(3<<28);
  //6. Initialize the I2C Master by writing the I2CMCR register with a value of 0x0000.0010.
	I2C1_MCR_R |= I2C_MCR_MFE;
  //7. Set the desired SCL clock speed of 100 Kbps by writing the I2CMTPR register with the correct
  //value. The value written to the I2CMTPR register represents the number of system clock periods
  //in one SCL clock period. The TPR value is determined by the following equation:
  //TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
  //TPR = (16MHz/(2*(6+4)*100000))-1;
  //TPR = 7
  //Write the I2CMTPR register with the value of 0x0000.0007.
	I2C1_MTPR_R |= (7<<0);
}

// Configures Master for Transmit or Recieve.
// Inputs: 1 (read) or 0 (write)
// Outputs: none
void RW(uint8_t bit){
  if (bit == 1){
	  I2C1_MSA_R |= (1<<0);
		return;
	}
	if (bit == 0){
	  I2C1_MSA_R &= ~(1<<0);
		return;
	}
}

// Set slave address.
// Inputs: slave address
// Outputs: none
void Set_Slave_Addr(uint8_t addr){
  I2C1_MSA_R |= (addr<<1);
}

// Master sends a byte of data.
// Inputs: data to transmit, 
//         conditions: generate  master enable, start, and/or stop bit, etc.
// Outputs: error code. Look at TM4C123 data sheet for further explanation. 
uint8_t Send_Byte(uint8_t data, uint8_t conditions){
  I2C1_MDR_R = data&0xFF; // input data
	I2C1_MCS_R = conditions&0x1F; // set contitions
	
	while ((I2C1_MCS_R&I2C_MCS_BUSY) != 0){}; // wait until controller is idle
	
	if ((I2C1_MCS_R&I2C_MCS_ERROR) != 0){ // check for errors
	  I2C1_MCS_R |= I2C_MCS_STOP; // generate stop bit
		return (I2C1_MCS_R&(I2C_MCS_ADRACK|I2C_MCS_DATACK|I2C_MCS_ERROR));
	}
}

// Master receives a byte of data.
// Inputs: address of variable to save received data, 
//         conditions: generate  master enable, start, and/or stop bit, etc.
// Outputs: error code. Look at TM4C123 data sheet for further explanation.
uint8_t Read_Byte(uint8_t *data, uint8_t conditions){
  I2C1_MCS_R = conditions&0x1F; // set contitions
	
	while ((I2C1_MCS_R&I2C_MCS_BUSY) != 0){}; // wait until controller is idle
		
	if ((I2C1_MCS_R&I2C_MCS_ERROR) != 0){ // check for errors
	  I2C1_MCS_R |= I2C_MCS_STOP; // generate stop bit
		return (I2C1_MCS_R&I2C_MCS_ERROR);
	}
	
	(*data) = I2C1_MDR_R&0XFF; // read data
}
