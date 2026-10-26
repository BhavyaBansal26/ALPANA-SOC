/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: gyroi2c.c 
 * Brief Description of file    : Measurement of Coordinates x,y,z with the usage of i2c 
 * Name of Author    	        : Kotteeswaran
 * Email ID                     : kottee.1@gmail.com
 
 Copyright (C) 2019  IIT Madras. All rights reserved.

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

******************************************************************************/
/**
@file gyroi2c.c
@brief Measurement of Coordinates x,y,z with the usage of i2c 
@detail 
*/ 

#include"gpio_i2c.h"

#define I2C_WRITE 0
#define I2C_READ 1

#define MPU6050_SLAVE_ADDRESS      0xD0
#define MPU_6050_OUTPUT_BEGIN      0x3B
#define MPU6050_DLPF_CFG           0x20
#define MPU6050_SMPL_DIV           0x19
#define MPU6050_ADDRESS            0x68 
#define DEV_IDENTITY               0X68
#define DEV_IDENTITY_DATA_REG      0X75
#define MPU6050_REG_PWR_MGMT       0X6B
#define MPU6050_RESET              0x80
#define MPU6050_ACC_SENS_8G        0x10
#define MPU6050_REG_GYRO_CONFIG    0x1B // Gyroscope Configuration
#define MPU6050_REG_ACCEL_CONFIG   0x1C // Accelerometer Configuration
#define MPU6050_GYRO_SENS_500D     0x08

#define PI 3.141592654

char readbuf[20];
int AccX,AccY,AccZ;
int accAngleX, accAngleY, GyroAngleX, GyroAngleY, GyroAngleZ;
int roll, pitch, yaw;
int AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
int AccErrorX,AccErrory;
int GyroX, GyroY, GyroZ;
int c = 0;
int i=0;
static unsigned char delay = 100;

//   soft float library need to be added, if hardfloat not there
 
 /** @fn void measuring value()
 * @brief  determines the x,y,z axes in terms of 16-bit
 * @details  In addition to measurements error adjustments is also made.  
 */
void measuring_value()
{
	// read accelerometer data
	I2cSendSlaveAddress(MPU6050_SLAVE_ADDRESS, I2C_WRITE, delay);//selecting slave to be read
	I2cWriteData(MPU_6050_OUTPUT_BEGIN, delay);//selecting register to be read
	I2cSendSlaveAddress(MPU6050_SLAVE_ADDRESS, I2C_READ, delay);
	I2c_shakti_readbytes(readbuf,14,1, delay);//to read the output values
	printf("helloworld inside measuring value\n");

	AccX = (readbuf[0]<<8 |readbuf[1]) / 4096; //16-bit X-axis data
	AccY = (readbuf[2]<<8 |readbuf[3]) / 4096; //16-bit Y-axis data
	AccZ = (readbuf[4]<<8 |readbuf[5]) / 4096; //16-bit Z-axis data

	GyroX = (readbuf[8]<<8 |readbuf[9]) / 65.5 ; 
	GyroY = (readbuf[10]<<8 |readbuf[11]) / 65.5; 
	GyroZ = (readbuf[12]<<8 |readbuf[13]) / 65.5;

	// Correct the outputs with the calculated error values
	GyroX = GyroX + GyroErrorX ;   // GyroErrorX
	GyroY = GyroY - GyroErrorY;    // GyroErrorY
	GyroZ = GyroZ + GyroErrorZ;    // GyroErrorZ
}

/** @fn int main()
 * @brief contains function calls for configuring different parameters.
 * @details determines whether the initalisation as well as verfication is done properly or not 
 * @return int
 */
int main()
{
	I2cInit();//configuring data and clocks as output lines

	printf("MPU6050 i2c Init completed\n");

	if (DEV_IDENTITY == I2c_Read_byte(MPU6050_SLAVE_ADDRESS, DEV_IDENTITY_DATA_REG, delay)){
		printf("DEVICE SUCCESFULLY VERIFIED");
	}
	else{
		printf("something went wrong , device identity not verified");
	}
	//configuring power management register
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_REG_PWR_MGMT, MPU6050_RESET, delay);

	delay_loop(1000,1000);   // we can use waitfor() function also (delay time to reset the device and initialisations proper)

	//setting internal clock of MPU6050
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_REG_PWR_MGMT, 0x00, delay);

	// configuring accelerometer -set sensitivity scale factor +-8g
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_REG_ACCEL_CONFIG, MPU6050_ACC_SENS_8G, delay);

	// configuring gyroscope-set  sensitivity scale factor to +-500deg/sec
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_SENS_500D, delay);

	// configuirng gyroscope output rate as 1khz
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_DLPF_CFG,0x01, delay);

	// configuring sample divider to 99(i.e sample rate= gyroscope output rate /(1+sample divider))
	I2c_Write_byte(MPU6050_SLAVE_ADDRESS, MPU6050_SMPL_DIV, 0x63, delay);//set sample rate to 10

	while (1){
		measuring_value();
		printf("AccErrorX: %d,AccErrMPU6050_SLAVE_ADDRESSorX");
		printf("AccErrorY: %d,AccErrorY");
		printf("GyroErrorX: %d,GyroErrorX");
		printf("GyroErrorY: %d,GyroErrorY");
		printf("GyroErrorZ %d,GyroErrorZ");
	}

	return 0;
}

/** @fn void calculate_imu_error()
 * @brief 
 * @details 
 * @warning  
 */
void calculate_imu_error()
{
	int c = 0;
	int yaw;
	float accAngleX, accAngleY, GyroAngleX, GyroAngleY, GyroAngleZ;

	while (c < 200){
		measuring_value();

		AccErrorX = AccErrorX + (atan(AccY / sqrt(pow(AccX, 2)) + pow(AccZ, 2))) * 180 / PI;
		AccErrorY = AccErrorY + (atan(AccY / sqrt(pow(AccX, 2)) + pow(AccZ, 2))) * 180 / PI;

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

	GyroX = GyroX - GyroErrorX ;     // GyroErrorX
	GyroY = GyroY - GyroErrorY;      // GyroErrorY
	GyroZ = GyroZ - GyroErrorZ;      // GyroErrorZ
}
