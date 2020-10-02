/***************************************************************************//**
 * @file
 * @brief MPL3115A2.c
 ******************************************************************************/

#include <stdio.h>

#include "em_i2c.h"

#include "thunderboard/util.h"
#include "thunderboard/board_4166.h"

#include "MPL3115A2.h"

void MPL3115A2_readRegister(uint8_t registerAddress, uint8_t* read_to, uint8_t read_length)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  // Setting LED to indicate transfer
  BOARD_ledSet(0x01);

  // Initializing I2C transfer
  i2cTransfer.addr          = MPL3115A2_I2C_BUS_ADDRESS << 1;
  i2cTransfer.flags         = I2C_FLAG_WRITE_READ;
  i2cTransfer.buf[0].data   = &registerAddress;
  i2cTransfer.buf[0].len    = 1;
  i2cTransfer.buf[1].data   = read_to;
  i2cTransfer.buf[1].len    = read_length;
  result = I2C_TransferInit(I2C0, &i2cTransfer);

  // Sending data
  while (result == i2cTransferInProgress)
  {
    result = I2C_Transfer(I2C0);
  }

  // Clearing pin to indicate end of transfer
  BOARD_ledSet(0x00);
}

void MPL3115A2_writeRegister(uint8_t registerAddress, uint8_t* write_array, uint8_t write_length)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  uint8_t writeArray[write_length + 1];
  uint8_t i;
  writeArray[0] = registerAddress;
  for(i = 0; i < write_length; i++)
  {
	  writeArray[i + 1] = write_array[i];
  }

  // Setting LED to indicate transfer
  BOARD_ledSet(0x01);

  // Initializing I2C transfer
  i2cTransfer.addr          = MPL3115A2_I2C_BUS_ADDRESS << 1;
  i2cTransfer.flags         = I2C_FLAG_WRITE_WRITE;
  i2cTransfer.buf[0].data   = writeArray;
  i2cTransfer.buf[0].len    = (write_length + 1);
  i2cTransfer.buf[1].data   = NULL;
  i2cTransfer.buf[1].len    = 0;
  result = I2C_TransferInit(I2C0, &i2cTransfer);

  // Sending data
  while (result == i2cTransferInProgress)
  {
    result = I2C_Transfer(I2C0);
  }

  // Clearing pin to indicate end of transfer
  BOARD_ledSet(0x00);
}

uint8_t MPL3115A2_readWhoAmI(void)
{
	uint8_t whoAmI = 0;
	MPL3115A2_readRegister(MPL3115A2_WHO_AM_I_ADDRESS, &whoAmI, 1);
	return whoAmI;
}

// Set the MPL3115A2 sensor to Altimeter mode
void MPL3115A2_setAltimeterMode(void)
{
	  uint8_t registerAddress = 0;
	  uint8_t registerValue = 0;
	  uint8_t ctrlReg1 = 0;

	  /* Set to Altimeter with an OSR = 128 */
	  // IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB8);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = 0xB8;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);

	  /* Enable Data Flags in PT_DATA_CFG */
	  //IIC_RegWrite(SlaveAddressIIC, 0x13, 0x07);
	  registerAddress = MPL3115A2_PT_DATA_CFG;
	  registerValue = 0x07;
	  MPL3115A2_writeRegister(registerAddress, &registerValue, 1);

	  /* Set Active */
	  //IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB9);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = ctrlReg1 | 0x01;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);
}

// Set the MPL3115A2 sensor to Barometer mode
void MPL3115A2_setBarometerMode(void)
{

	  uint8_t registerAddress = 0;
	  uint8_t registerValue = 0;
	  uint8_t ctrlReg1 = 0;

	  /* Set to Altimeter with an OSR = 128 */
	  // IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB8);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = 0x38;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);

	  /* Enable Data Flags in PT_DATA_CFG */
	  //IIC_RegWrite(SlaveAddressIIC, 0x13, 0x07);
	  registerAddress = MPL3115A2_PT_DATA_CFG;
	  registerValue = 0x07;
	  MPL3115A2_writeRegister(registerAddress, &registerValue, 1);

	  /* Set Active */
	  //IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB9);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = ctrlReg1 | 0x01;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);
}

void MPL3115A2_measureAltitudeAndTemperature(int32_t* resultAltitude, int16_t* resultTemperature)
{

	  uint8_t timeout;

	  uint8_t registerAddress = 0;
	  uint8_t statusReg = 0;
	  uint8_t measurementsAddress = MPL3115A2_OUT_P_MSB;

	  uint8_t measurements[5] = {0};

	  int32_t altitude = 0;
	  int16_t temperature = 0;

	  /* Read STATUS Register */
	  //STA = IIC_RegRead(SlaveAddressIIC, 0x00);
	  /* Wait for data to become ready */
	  registerAddress = MPL3115A2_STATUS;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &statusReg, 1);

	    /* Is Data Ready */
	    //(STA & 0x08) == TRUE
	    if ( (statusReg & 0x08) == 8 ) {
	      /* Data is ready */
		    /* Read OUT_P and OUT_T */
		    /* This clears the DRDY Interrupt */
	    	//  OUT_P_MSB = IIC_RegRead(SlaveAddressIIC, 0x01);
	    	//  OUT_P_CSB = IIC_RegRead(SlaveAddressIIC, 0x02);
	    	//  OUT_P_LSB = IIC_RegRead(SlaveAddressIIC, 0x03);
	    	//  OUT_T_MSB = IIC_RegRead(SlaveAddressIIC, 0x04);
	    	//  OUT_T_LSB = IIC_RegRead(SlaveAddressIIC, 0x05);
	    	MPL3115A2_readRegister(measurementsAddress, measurements, 5);

			#if DEBUG_MODE == 1
	    		printf("measurements: %d %d %d %d %d\r\n", measurements[0], measurements[1], measurements[2], measurements[3], measurements[4]);
			#endif

			altitude = (measurements[0] << 24);
			altitude |= (measurements[1] << 16);
			altitude |= (measurements[2] << 8);

			*resultAltitude = altitude;

			temperature = measurements[3];
			temperature <<= 8;
			temperature |= measurements[4];
			temperature >>= 4;

			if (temperature & 0x800) {
			  temperature |= 0xF000;
			}

			*resultTemperature = temperature;

			#if DEBUG_MODE == 1
				float alt = altitude;
				alt /= 65536.0;
				float temp = temperature;
				temp /= 16.0;
				printf("Altitude: %d.%d%d m\r\n", (int) (altitude), (int) (altitude * 10.0f) % 10, (int) (altitude * 100.0f) % 10);
				printf("Temperature: %d.%d%d C\r\n", (int) (temp), (int) (temp * 10.0f) % 10, (int) (temp * 100.0f) % 10);
			#endif

	      break;
	    } else {
	      UTIL_delay(10);
	    }
	  }
}

void MPL3115A2_measurePressureAndTemperature(uint32_t* resultPressure, int16_t* resultTemperature)
{
	  uint8_t timeout;

	  uint8_t registerAddress = 0;
	  uint8_t statusReg = 0;
	  uint8_t measurementsAddress = MPL3115A2_OUT_P_MSB;

	  uint8_t measurements[5] = {0};

	  uint32_t pressure = 0;
	  int16_t temperature = 0;

	  /* Read STATUS Register */
	  //STA = IIC_RegRead(SlaveAddressIIC, 0x00);
	  /* Wait for data to become ready */
	  registerAddress = MPL3115A2_STATUS;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &statusReg, 1);

	    /* Is Data Ready */
	    //(STA & 0x08) == TRUE
	    if ( (statusReg & 0x08) == 8 ) {
	      /* Data is ready */
		    /* Read OUT_P and OUT_T */
		    /* This clears the DRDY Interrupt */
	    	//  OUT_P_MSB = IIC_RegRead(SlaveAddressIIC, 0x01);
	    	//  OUT_P_CSB = IIC_RegRead(SlaveAddressIIC, 0x02);
	    	//  OUT_P_LSB = IIC_RegRead(SlaveAddressIIC, 0x03);
	    	//  OUT_T_MSB = IIC_RegRead(SlaveAddressIIC, 0x04);
	    	//  OUT_T_LSB = IIC_RegRead(SlaveAddressIIC, 0x05);
	    	MPL3115A2_readRegister(measurementsAddress, measurements, 5);

			#if DEBUG_MODE == 1
	    		printf("measurements: %d %d %d %d %d\r\n", measurements[0], measurements[1], measurements[2], measurements[3], measurements[4]);
			#endif

			pressure = measurements[0]; // MSB
			pressure <<= 8;
			pressure |= measurements[1]; // CSB
			pressure <<= 8;
			pressure |= measurements[2]; // LSB
			pressure >>= 4;

			*resultPressure = pressure;

			temperature = measurements[3];
			temperature <<= 8;
			temperature |= measurements[4];
			temperature >>= 4;

			if (temperature & 0x800) {
			  temperature |= 0xF000;
			}

			*resultTemperature = temperature;

			#if DEBUG_MODE == 1
				float alt = altitude;
				alt /= 65536.0;
				float temp = temperature;
				temp /= 16.0;
				printf("Pressure: %d.%d%d m\r\n", (int) (pressure), (int) (pressure * 10.0f) % 10, (int) (pressure * 100.0f) % 10);
				printf("Temperature: %d.%d%d C\r\n", (int) (temp), (int) (temp * 10.0f) % 10, (int) (temp * 100.0f) % 10);
			#endif

	      break;
	    } else {
	      UTIL_delay(10);
	    }
	  }
}

void MPL3115A2_measureOneShotInBarometerMode(void)
{

	  uint8_t timeout;

	  uint8_t registerAddress = 0;
	  uint8_t ctrlReg1 = 0;
	  uint8_t statusReg = 0;
	  uint8_t measurementsAddress = MPL3115A2_OUT_P_MSB;
	  uint8_t measurements[5] = {0};
	  uint32_t pressure = 0;
	  uint32_t temperature = 0;

	  registerAddress = MPL3115A2_CTRL_REG1;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &ctrlReg1, 1);
		if(ctrlReg1 & MPL3115A2_CTRL_REG1_OST) {
			UTIL_delay(10);
		}
		else {
			break;
		}
	  }

	  /* Set to Altimeter with an OSR = 128 */
	  // IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB8);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = 0x3A;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);

	  registerAddress = MPL3115A2_STATUS;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &statusReg, 1);
		if(statusReg & MPL3115A2_REGISTER_STATUS_PDR ) {
			break;
		}
		else {
			UTIL_delay(10);
		}
	  }

		MPL3115A2_readRegister(measurementsAddress, measurements, 5);

		pressure = (measurements[0] << 8);
		pressure |= (measurements[1] << 8);
		pressure |= (measurements[2] >> 4);

//		float baro = pressure;
//		baro /= 4.0;

		temperature = measurements[3];
		temperature <<= 8;
		temperature |= measurements[4];
		temperature >>= 4;

		if (temperature & 0x800) {
		  temperature |= 0xF000;
		}

		printf("Barometer: %lu.%lu pascal, %lu.%lu C\r\n", pressure >> 2, pressure % 4, temperature >> 4, temperature % 16);

//		float temp = temperature;
//		temp /= 16.0;

		//printf("Temperature: %d.%d%d C\r\n", (int) (temp), (int) (temp * 10.0f) % 10, (int) (temp * 100.0f) % 10);
		//printf("Pressure: %d.%d%d p\r\n", (int) (baro), (int) (baro * 10.0f) % 10, (int) (baro * 100.0f) % 10);
}

void MPL3115A2_measureOneShotInAltimeterMode(void)
{
	  uint8_t timeout;

	  uint8_t registerAddress = 0;
	  uint8_t ctrlReg1 = 0;
	  uint8_t statusReg = 0;
	  uint8_t measurementsAddress = MPL3115A2_OUT_P_MSB;
	  uint8_t measurements[5] = {0};
	  int32_t altitude = 0;
	  uint32_t temperature = 0;

	  registerAddress = MPL3115A2_CTRL_REG1;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &ctrlReg1, 1);
		if(ctrlReg1 & MPL3115A2_CTRL_REG1_OST) {
			UTIL_delay(10);
		}
		else {
			break;
		}
	  }

	  /* Set to Altimeter with an OSR = 128 */
	  // IIC_RegWrite(SlaveAddressIIC, 0x26, 0xB8);
	  registerAddress = MPL3115A2_CTRL_REG1;
	  ctrlReg1 = 0xBA;
	  MPL3115A2_writeRegister(registerAddress, &ctrlReg1, 1);

	  registerAddress = MPL3115A2_STATUS;
	  timeout = 10;
	  while ( timeout-- ) {
		MPL3115A2_readRegister(registerAddress, &statusReg, 1);
		if(statusReg & MPL3115A2_REGISTER_STATUS_PDR ) {
			break;
		}
		else {
			UTIL_delay(10);
		}
	  }

		MPL3115A2_readRegister(measurementsAddress, measurements, 5);

		altitude = (measurements[0] << 24);
		altitude |= (measurements[1] << 16);
		altitude |= (measurements[2] << 8);

//		float alt = altitude;
//		alt /= 65536.0;

		temperature = measurements[3];
		temperature <<= 8;
		temperature |= measurements[4];
		temperature >>= 4;

		if (temperature & 0x800) {
		  temperature |= 0xF000;
		}

		printf("Altimeter: %ld.%ld m, %lu.%lu C\r\n", altitude >> 16, altitude % 65536, temperature >> 4, temperature % 16);

//		float temp = temperature;
//		temp /= 16.0;

		//printf("Temperature: %d.%d%d C\r\n", (int) (temp), (int) (temp * 10.0f) % 10, (int) (temp * 100.0f) % 10);
		//printf("Altitude: %d.%d%d p\r\n", (int) (alt), (int) (alt * 10.0f) % 10, (int) (alt * 100.0f) % 10);
}
