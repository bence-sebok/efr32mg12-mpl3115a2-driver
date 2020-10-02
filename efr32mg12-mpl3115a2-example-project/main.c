/***************************************************************************//**
 * @file
 * @brief main.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "retargetserial.h"

#include "thunderboard/util.h"
#include "thunderboard/board.h"
#include "thunderboard/board_4166.h"

#include "init_mcu.h"

#include "MPL3115A2.h"

#include "em_i2c.h"
#include "em_cmu.h"

// Set this macro to 1 for displaying detailed debug informations
#define DEBUG_MODE (0)
// Set the macro to 1 for using the MPL3115A2 sensor in Altimeter mode
#define MPL3115A2_ALTIMETER_MODE (0)

/**************************************************************************//**
 * @brief  Setup I2C peripheral
 *****************************************************************************/
void initI2C(void)
{
  // Using default settings
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
  // Use ~400khz SCK
  i2cInit.freq = I2C_FREQ_FAST_MAX;

  // Enabling clock to the I2C, GPIO
  printf("Enabling clocks to the I2C, GPIO\r\n");
  CMU_ClockEnable(cmuClock_I2C0, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Using PC10 (SDA) and PC11 (SCL)
  printf("Enable I2C pins: using PC10 (SDA) and PC11 (SCL)\r\n");
  GPIO_PinModeSet(gpioPortC, 10, gpioModeWiredAnd, 1);
  GPIO_PinModeSet(gpioPortC, 11, gpioModeWiredAnd, 1);

  // Enable pins at location 15 as specified in datasheet
  printf("Enable pins at location 15 as specified in datasheet\r\n");
  I2C0->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
  I2C0->ROUTELOC0 = (I2C0->ROUTELOC0 & (~_I2C_ROUTELOC0_SDALOC_MASK)) | I2C_ROUTELOC0_SDALOC_LOC15;
  I2C0->ROUTELOC0 = (I2C0->ROUTELOC0 & (~_I2C_ROUTELOC0_SCLLOC_MASK)) | I2C_ROUTELOC0_SCLLOC_LOC15;

  // Initializing the I2C
  I2C_Init(I2C0, &i2cInit);
}

void initModule(void)
{
  /**************************************************************************/
  /* Module init                                                            */
  /**************************************************************************/
  UTIL_init();
  BOARD_init();
  RETARGET_SerialInit();
  return;
}

uint8_t initMPL3115A2(void)
{
	uint8_t whoAmI = 0;
	printf("Reading WHO_AM_I register from MPL3115A2...\r\n");
	whoAmI = MPL3115A2_readWhoAmI();
	printf("MPL3115A2 WHO_AM_I: 0x%2X --> %s\r\n", whoAmI, whoAmI == MPL3115A2_WHO_AM_I_VALUE ? "OK" : "FAILURE");
	if(whoAmI == MPL3115A2_WHO_AM_I_VALUE) {
	  BOARD_ledSet(0x01); // Turn the green LED ON
	  return 0; // success
	}
	else {
	  BOARD_ledSet(0x02); // Turn the red LED ON
	  return 1; // failure
	}
}

int main(void)
{
	/**************************************************************************/
	/* Local variables of the main function                                   */
	/**************************************************************************/
	uint8_t status = 1;
	int32_t altitude = 0;
	int16_t temperature = 0;
	uint32_t pressure = 0;

	/**************************************************************************/
	/* Device errata init                                                     */
	/**************************************************************************/
	initMcu();

	/**************************************************************************/
	/* Board init                                                             */
	/**************************************************************************/
	initModule();

	/**************************************************************************/
	/* I2C peripheral init                                                    */
	/**************************************************************************/
	printf("initI2C...\r\n");
	initI2C();
	printf("initI2C: DONE\r\n");

	/**************************************************************************/
	/* MPL3115A2 init                                                         */
	/**************************************************************************/
	printf("initMPL3115A2...\r\n");
	status = initMPL3115A2();
	printf("initMPL3115A2: %s\r\n", status == 0 ? "DONE" : "FAILED");

	/**************************************************************************/
	/* Set the mode of the MPL3115A2 sensor                                   */
	/**************************************************************************/
	#if MPL3115A2_ALTIMETER_MODE == 1
		printf("Set the MPL3115A2 sensor to Altimeter mode\r\n");
		MPL3115A2_setAltimeterMode();
	#else
		printf("Set the MPL3115A2 sensor to Barometer mode\r\n");
		MPL3115A2_setBarometerMode();
	#endif

	/**************************************************************************/
	/* Application loop                                                       */
	/**************************************************************************/
	while (1) {
		printf("\r\nMPL3115A2 measure\r\n");
		#if MPL3115A2_ALTIMETER_MODE == 1
			MPL3115A2_measureAltitudeAndTemperature(&altitude, &temperature);
		#else
			MPL3115A2_measurePressureAndTemperature(&pressure, &temperature);
		#endif
		#if MPL3115A2_ALTIMETER_MODE == 1
			printf("Altimeter mode:\r\n");
			printf("Altitude: %ld.%ld meter\r\n", altitude >> 16, altitude % 65536);
		#else
			printf("Pressure: %lu.%lu Pascal\r\n", pressure >> 2, pressure % 2);
		#endif
		printf("Temperature: %d.%d C\r\n", temperature >> 4, temperature % 16);
		printf("---------------\r\n");
		UTIL_delay(3000);
	}
}
