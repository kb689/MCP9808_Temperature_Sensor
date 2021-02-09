// TempSensor.c
// Runs on TM4C123.
// Temperature(deg Farenheight) is displayed on a terminal(Putty) via UART0.
// Uses MCP9808 temperature sensor.
// TM4C pins:
// PA6: SCL
// PA7: SDA
// SCL and SDA connected to 2.2k ohm pull-up resistors.
// Kirabo Nsereko
// 1/6/2021

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "I2C.h"
#include "UART0.h"


#define SLAVE_ADDR     0x18 // address of MCP3808
#define CONFIG_R       0x01 // address of MCP2808 Configuration register
#define T_UPPER_R      0x02 // address of MCP3808 T_upper register
#define T_LOWER_R      0x03 // address of MCP3808 T_lower register
#define ABMIENT_TEMP_R 0x05 // address of MCP3080 Ambient temp register


void Delay10us(uint32_t time);// Approximate 10.1us delay.
void Lower_Temp_Boundary(uint16_t temp); // Sets lower temp boundary in T_lower register.
void Sensor_Config(uint16_t config); // Configures sensor.
uint16_t Read_Register(uint8_t regAddr); // Reads a register.

// Calculates temperature from upper and lower bytes read from MCP3808 Ambient Temp register.     
void Calculate_Temp(uint8_t *upperByte, uint8_t *lowerByte, int32_t *temperature);


int main(void){
	uint8_t upperByte;
	uint8_t lowerByte;
  int32_t temperature; //Temperature = Ambient Temperature (°F)
	
	UART0_Init();
  I2C1_Init();
	Set_Slave_Addr(SLAVE_ADDR);
	
	while (1){
	// Read temp from Ambient Temp register
	RW(0); // transmit
		
	Send_Byte(ABMIENT_TEMP_R, I2C_MCS_RUN|I2C_MCS_START); // write to Ambient temp reg. Run, start
	RW(1); // receive
	Read_Byte(&upperByte, I2C_MCS_ACK|I2C_MCS_START|I2C_MCS_RUN); // repeated start, ack, run
	Read_Byte(&lowerByte, I2C_MCS_RUN|I2C_MCS_STOP); // nak, run, stop
	  
  Calculate_Temp(&upperByte, &lowerByte, &temperature); //Convert the temperature data 

	UART0_OutString("Temperature = ");
	if (temperature){
	  UART0_OutUDec(temperature);
		UART0_OutString(" deg ");
		UART0_OutChar('F');
		UART0_OutChar('\n');
		UART0_OutChar('\r');
	} else{
		UART0_OutChar('-');
    UART0_OutUDec(temperature);
		UART0_OutString(" deg ");
		UART0_OutChar('F');
		UART0_OutChar('\n');
		UART0_OutChar('\r');
	}

	Delay10us(100000);
		
	}		
}

// Configures sensor.
// Config:
// Interrupt output
// Active-low alert output polarity (pullup resistor required)
// Alert output for T_upper, T_lower, and T_crit
// Alert output enabled
// Inputs: configuration (see MCP9808 datasheet)
// Outputs: none
void Sensor_Config(uint16_t config){
  RW(0); // write
	Send_Byte(CONFIG_R, I2C_MCS_RUN|I2C_MCS_START); // Run, start
	Send_Byte((config&0xFF00)>>8, I2C_MCS_RUN); // Send first config byte. Run, stop
	Send_Byte(config&0x00FF, I2C_MCS_RUN|I2C_MCS_STOP); // Send last config byte. Run, stop
}

// Sets lower temp boundary in T_lower register.
// Inputs: temperature boundary (must be positive whole number between 0-125 deg celcius).
// Outputs: none
void Lower_Temp_Boundary(uint16_t temp){
	temp = temp<<4;
  RW(0); // write
	Send_Byte(T_LOWER_R, I2C_MCS_RUN|I2C_MCS_START); // access T_lower register. Run, start
	Send_Byte((temp&0xFF00)>>8, I2C_MCS_RUN); // send MSB. Run
	Send_Byte(temp&0x00FF, I2C_MCS_RUN|I2C_MCS_STOP); // send LSB. Run, stop
}

// Reads a register.
// Inputs: register address
// Outputs: register value
uint16_t Read_Register(uint8_t regAddr){
	uint8_t upperByte;
	uint8_t lowerByte;
	uint16_t regVal = 0;

	RW(0); // transmit
		
	Send_Byte(regAddr, I2C_MCS_RUN|I2C_MCS_START); // write to Config reg. Run, start
	RW(1); // receive
	Read_Byte(&upperByte, I2C_MCS_ACK|I2C_MCS_START|I2C_MCS_RUN); // repeated start, ack, run
	Read_Byte(&lowerByte, I2C_MCS_RUN|I2C_MCS_STOP); // nak, run, stop
	regVal |= ((upperByte<<8)|lowerByte);
	return regVal;
}


// Calculates temperature from upper and lower bytes read from MCP3808 Ambient Temp register.
// Inputs: address of upperByte and lowerByte read from MCP3808, and temperature variable
// Outputs: none        
void Calculate_Temp(uint8_t *upperByte, uint8_t *lowerByte, int32_t *temperature){
	float tempC; // temperature in °C
  //First Check flag bits
  if (((*upperByte) & 0x80) == 0x80){ //TA ³ TCRIT
  }
	
  if (((*upperByte) & 0x40) == 0x40){ //TA > TUPPER
  }
	
  if (((*upperByte) & 0x20) == 0x20){ //TA < TLOWER
	}
	
  (*upperByte) = (*upperByte) & 0x1F; //Clear flag bits
	
  if (((*upperByte) & 0x10) == 0x10){ //TA < 0°C
    (*upperByte) = (*upperByte) & 0x0F; //Clear SIGN
    tempC = 256 - ((*upperByte)*16 + ((float)(*lowerByte)) / 16);
		(*temperature) = (int32_t)((tempC*(9.0/5)) + 32); // convert temp to farenheight
  }else //TA ³ 0°C
    tempC = ((*upperByte)*16 + ((float)(*lowerByte)) / 16); // temp in °C
	  (*temperature) = (int32_t)((tempC*(9.0/5)) + 32); // convert temp to farenheight
}

// Approximate 10.1us delay.
// variable 'i' calculated from eqn: delay = i*clock period*6cycles.
// Value of 'i' adjusted through use of logic analyzer to get more accurate delay.
// i=37 @ 16MHz clock rate. 
// Input: timescale factor. E.g. if time = 2, delay = 10*2us.
// Output: none
void Delay10us(uint32_t time){
  uint32_t i;
	while(time){
	  i = 37; 
		while(i){
		  i--;
		}
		time--;
	}
}
