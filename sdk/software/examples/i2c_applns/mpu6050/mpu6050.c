/*************************************************************************
 * Project           	       : shakti devt board
 * Name of the file	       : mpu6050.c
 * Brief Description of file   : Contains the driver routines to configure and read mpu6050.
 * Name of Author              : Kotteeswaran
 * Email ID                    : kottee.1@gmail.com

 Copyright (C) 2020  IIT Madras. All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
***************************************************************************/
/**
@file   mpu6050.c
@brief  Contains the driver routines to write and read all locations of mpu6050.
@detail I2C based routines to write and read all locations of mpu6050.
*/

#include <stdint.h>//Includes the definitions of standard input/output functions//
#include "i2c.h"
#include "log.h"
#include "uart.h"

#define DELAY1 100
#define DELAY2 50


#define DELAY_VALUE 900
#if 1
#define PRESCALER_COUNT 24
#define SCLK_COUNT 9
#else
#define PRESCALER_COUNT 8
#define SCLK_COUNT 6
#endif
#define I2C i2c_instance[1]
#define RD_DATA_CNT 14

#define MPU6050_SLAVE_ADDRESS 0xD0
#define MPU_6050_OUTPUT_BEGIN 0x3B
#define MPU6050_DLPF_CFG 0x20
#define MPU6050_SMPL_DIV 0x19
#define MPU6050_ADDRESS   0x68
#define DEV_IDENTITY  0X68
#define DEV_IDENTITY_DATA_REG 0X75
#define MPU6050_REG_PWR_MGMT 0X6B
#define MPU6050_RESET         0x80
#define MPU6050_ACC_SENS_2G         0x00
#define MPU6050_ACC_SENS_4G         0x08
#define MPU6050_ACC_SENS_8G         0x10
#define MPU6050_REG_GYRO_CONFIG      0x1B // Gyroscope Configuration
#define MPU6050_REG_ACCEL_CONFIG     0x1C // Accelerometer Configuration
#define MPU6050_GYRO_SENS_500D       0x08

#define PI 3.141592654

char readbuf[RD_DATA_CNT];
int AccX,AccY,AccZ;
int accAngleX, accAngleY, GyroAngleX, GyroAngleY, GyroAngleZ;
int roll, pitch, yaw;
int AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
int AccErrorX,AccErrory;
int GyroX, GyroY, GyroZ;
int c = 0;
int i=0;

int read_mpu6050_register(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned char *read_data, unsigned char length, unsigned long delay)
{
	unsigned char read_buf[4] = {'\0'};
	int i = 0, j = 0,  k = 0, status=0;
	unsigned char temp = 0;

//Writes the slave address for write
	i2c_send_slave_address(i2c_instance, MPU6050_SLAVE_ADDRESS, I2C_WRITE, 800);
//Writes the pointer to address that needs to be read
	i2c_write_data(i2c_instance, reg_offset, delay);
//Restarts the I2C transaction to start reading the temperature value.
	i2c_instance->control = I2C_REPSTART;
	delay_loop(DELAY1, DELAY2);
//Writes the slave address for read
	i2c_write_data(i2c_instance, MPU6050_SLAVE_ADDRESS | I2C_READ, delay);
/* Make a dummy read as per spec of the I2C controller */
	i2c_instance->control = I2C_NACK;
	i2c_read_data(i2c_instance, &temp, delay);
//	printf("\n Reading the data");

	i2c_read_data_nack(i2c_instance, &temp, delay);
	*read_data = temp;
//	delay_loop(DELAY1, DELAY2);
	i2c_instance->control = I2C_STOP;
	delay_loop(DELAY1, DELAY2);
	return 0;
}

/** @fn int read_mpu6050_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int *readTemp, unsigned char length, unsigned long delay)
 * @brief Reads the ADC value from PCF8591
 * @details Reads 4 ADC values from PCF8591 over I2C interface
 * @param i2c_struct*
 * @param reg_offset
 * @param *readTemp
 * @param length
 * @param delay
 * @return read status (zero on success)
 */
int read_mpu6050_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned char *readTemp, unsigned char length, unsigned long delay)
{
//	unsigned char read_buf[4] = {'\0'};
	int i = 0, j = 0,  k = 0, status=0;
	unsigned char temp = 0;

//Writes the slave address for write
	i2c_send_slave_address(i2c_instance, MPU6050_SLAVE_ADDRESS, I2C_WRITE, delay);
//Writes the pointer to address that needs to be read
	i2c_write_data(i2c_instance, reg_offset, delay);
//	delay_loop(DELAY1, DELAY2);
//Stops the I2C transaction to start reading the temperature value.
	i2c_instance->control = I2C_REPSTART;
	delay_loop(DELAY1, DELAY2);

//Writes the slave address for read
	i2c_write_data(i2c_instance, MPU6050_SLAVE_ADDRESS + I2C_READ, delay);
//	i2c_send_slave_address(i2c_instance, MPU6050_SLAVE_ADDRESS, I2C_READ, delay);

/* Make a dummy read as per spec of the I2C controller */
	i2c_read_data(i2c_instance, &temp, delay);

//Reads the MSB Byte of temperature [D9 - D1]
	for(i = 0; i < length; i++)
	{
		if(i == (length - 1) )
		{
			log_debug("\nSending nack");
			i2c_instance->control = I2C_NACK;
//			i2c_read_data_nack(i2c_instance, &temp, delay);
			i2c_read_data(i2c_instance, &temp, delay);
			break;
			}
		else
		{
			i2c_read_data(i2c_instance, &temp, delay);
		}
		log_debug("\n read data[%x]: %x", i, temp);
		*readTemp = temp;
		*readTemp++;
	}
	
	log_debug("\n Sending Stop condition");
	i2c_instance->control = I2C_STOP;
	return 0;
}




/** @fn int write_pcf8591_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int *write_value, unsigned char length, unsigned long delay)
 * @brief WRites into PCF8591 Register
 * @details WRites the passed value into passed PCF8591 Register (address) over I2C interface.
 * @param i2c_struct*
 * @param reg_offset
 * @param write_value
 * @param length
 * @param delay
 * @return Write status (Zero on success)
 */
int write_mpu6050_register(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int *write_value, unsigned long delay)
{
	i2c_instance->control = I2C_IDLE;
	delay_loop(DELAY1, DELAY2);
	i2c_send_slave_address(i2c_instance, MPU6050_SLAVE_ADDRESS, I2C_WRITE, delay);
//	delay_loop(DELAY1, DELAY2);
	i2c_write_data(i2c_instance, reg_offset, delay);
//	delay_loop(DELAY1, DELAY2);
	log_debug("\n write value: %x", *write_value);
	i2c_write_data(i2c_instance,  ( *write_value & 0xff), delay);
	//Stops the I2C transaction to start reading the temperature value.
//	delay_loop(DELAY1, DELAY2);
	i2c_instance->control = I2C_STOP;
	delay_loop(DELAY1, DELAY2);

}



/** @fn int write_pcf8591_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int *write_value, unsigned char length, unsigned long delay)
 * @brief WRites into PCF8591 Register
 * @details WRites the passed value into passed PCF8591 Register (address) over I2C interface.
 * @param i2c_struct*
 * @param reg_offset
 * @param write_value
 * @param length
 * @param delay
 * @return Write status (Zero on success)
 */
int write_mpu6050_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int *write_value, unsigned char length, unsigned long delay)
{
	int i = 0, j = 0,  k = 0, status=0;
	unsigned int temp = 0;
	
	i2c_send_slave_address(i2c_instance, MPU6050_SLAVE_ADDRESS, I2C_WRITE, delay);
	i2c_write_data(i2c_instance, reg_offset, delay);
	delay_loop(DELAY1, DELAY2);
	
	if(1 == length)
	{
		i2c_write_data(i2c_instance,  ( *write_value	& 0xff), delay);
	}
	else
	{
		for(i = 0; i < length; i++)
		{
			i2c_write_data(i2c_instance, (*write_value++ & 0xff), delay);

		}
	}

//Stops the I2C transaction to start reading the temperature value.
	i2c_instance->control = I2C_STOP;
	delay_loop(DELAY1, DELAY2);
	return 0;
}

/*
   soft float library need to be added, if hardfloat not there
 */
void measuring_value(unsigned long delay)
{
	// read accelerometer data
	read_mpu6050_registers(I2C, MPU_6050_OUTPUT_BEGIN, readbuf, RD_DATA_CNT, delay);
	printf("\n Read Buf: { ");
	for(i = 0; i < RD_DATA_CNT; i++)
	{
		log_info(" %x, ", readbuf[i] );
		}
	log_info(" }\n" ); 
/*	AccX = (readbuf[0]<<8 |readbuf[1]) / 4096; //16-bit X-axis data
	AccY = (readbuf[2]<<8 |readbuf[3]) / 4096; //16-bit Y-axis data
	AccZ = (readbuf[4]<<8 |readbuf[5]) / 4096; //16-bit Z-axis data

	GyroX = (readbuf[8]<<8 |readbuf[9]) /65.5 ; //
	GyroY = (readbuf[10]<<8 |readbuf[11]) / 65.5; //
	GyroZ = (readbuf[12]<<8 |readbuf[13]) / 65.5;

	// Correct the outputs with the calculated error values
	GyroX = GyroX + GyroErrorX ; // GyroErrorX
	GyroY = GyroY - GyroErrorY; // GyroErrorY
	GyroZ = GyroZ + GyroErrorZ; // GyroErrorZ
*/
}


void calculate_imu_error(unsigned long delay)
{
	int c=0;
	int yaw;
	float accAngleX, accAngleY, GyroAngleX, GyroAngleY, GyroAngleZ;

	while(c<200)
	{
		measuring_value(delay);

		AccErrorX = AccErrorX + (atan(AccY / sqrt(pow(AccX, 2)) + pow(AccZ, 2)))* 180 / PI;
		AccErrorY = AccErrorY + (atan(AccY / sqrt(pow(AccX, 2)) + pow(AccZ, 2)))* 180 / PI;

		GyroErrorX = GyroErrorX + (GyroX / 65.5);
		GyroErrorY = GyroErrorY + (GyroY / 65.5);
		GyroErrorZ = GyroErrorZ + (GyroZ / 65.5);
		c++;
	}
	c=0;
	AccErrorX = AccErrorX / 200;
	AccErrorY = AccErrorY / 200;

	accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX; // AccErrorX
	accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY; // AccErrory
	// sum all reading
	GyroErrorX = GyroErrorX / 200;
	GyroErrorY = GyroErrorY / 200;
	GyroErrorZ = GyroErrorZ / 200;

	GyroX = GyroX - GyroErrorX ; // GyroErrorX
	GyroY = GyroY - GyroErrorY; // GyroErrorY
	GyroZ = GyroZ - GyroErrorZ; // GyroErrorZ
}

unsigned char CheckForDeviceId(unsigned long delay)
{
	unsigned char readBuf = 0xFF;
	read_mpu6050_register(I2C, DEV_IDENTITY_DATA_REG, &readBuf, 1, delay);
	log_debug("\n Read Buf Value: %x", readBuf);
	return readBuf;
}

int main()
{
	unsigned char i = 0;
	int timeout;
	unsigned long delay = 500;
	unsigned int write_buf[7] = {0x00};
	unsigned char length = 0;

	log_debug("\tI2C: MPU6050 - ACC test\n");


	i2c_init();

	if(config_i2c(I2C, PRESCALER_COUNT, SCLK_COUNT))
	{
			log_error("\tSomething Wrong In Initialization\n");
			return -1;
	}
	else
		log_info("\tIntilization Happened Fine\n");

		if(DEV_IDENTITY == CheckForDeviceId(delay))
		{
			log_info("Device ID Check Success");
		}
		else
		{
			log_error("Could not verify Device identity");
		}
		write_buf[0] = 1;
		write_mpu6050_register(I2C, MPU6050_REG_PWR_MGMT, &write_buf[0],  delay);
		write_buf[0] = MPU6050_GYRO_SENS_500D;
		write_mpu6050_register(I2C, MPU6050_REG_GYRO_CONFIG, &write_buf[0], delay);
		write_buf[0] = 0x01;
		write_mpu6050_register(I2C, 0x1B, &write_buf[0], delay);
		write_buf[0] = 0x01;
		write_mpu6050_register(I2C, 0x1C, &write_buf[0], delay);
		write_buf[0] = 0x01;
		write_mpu6050_register(I2C, 0x6B, &write_buf[0], delay);


	log_debug("\n Entering While loop");
	while(1)
	{
		measuring_value(delay);
		delay_loop(2000, 1000);
	}

	return 0;
}

