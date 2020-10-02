/***************************************************************************//**
 * @file
 * @brief MPL3115A2.h
 ******************************************************************************/

#ifndef MPL3115A2_H
#define MPL3115A2_H

#include <stdlib.h>
#include <stdint.h>

// I2C address of the sensor on the bus
#define MPL3115A2_I2C_BUS_ADDRESS     (0x60) // I2C address of the sensor on the bus

/*
 * Register addresses of the sensor
 */
#define MPL3115A2_WHO_AM_I_ADDRESS    (0x0C) // WHO_AM_I Register address
#define MPL3115A2_STATUS 			  (0x00) // Sensor Status Register address
#define MPL3115A2_PT_DATA_CFG 		  (0x13) // PT Data Configuration Register address
#define MPL3115A2_CTRL_REG1 		  (0x26) // Control Register 1 address
#define MPL3115A2_OUT_P_MSB 		  (0x01) // Root pointer to Pressure and Temperature data register address

/*
 * Register offsets, default values
 */
#define MPL3115A2_WHO_AM_I_VALUE      (0xC4) // Default value of the WHO_AM_I register
#define MPL3115A2_CTRL_REG1_OST  	  (0x02) // Single measurement bit
#define MPL3115A2_REGISTER_STATUS_PDR (0x04) // Pressure Data Ready value

void MPL3115A2_readRegister(uint8_t registerAddress, uint8_t* read_to, uint8_t read_length);
void MPL3115A2_writeRegister(uint8_t registerAddress, uint8_t* write_array, uint8_t write_length);
uint8_t MPL3115A2_readWhoAmI(void);
void MPL3115A2_setAltimeterMode(void);
void MPL3115A2_setBarometerMode(void);
void MPL3115A2_measureAltitudeAndTemperature(int32_t* resultAltitude, int16_t* resultTemperature);
void MPL3115A2_measurePressureAndTemperature(uint32_t* resultPressure, int16_t* resultTemperature);
void MPL3115A2_measureOneShotInBarometerMode(void);
void MPL3115A2_measureOneShotInAltimeterMode(void);

#endif // MPL3115A2_H
