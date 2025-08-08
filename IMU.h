#ifndef IMU_H
#define IMU_H

#define i2c_handle "/dev/i2c-1"

#define MPU9250_ADDRESS 0x68
#define AK8963_SLAVE_ADDRESS 0x0C
#define BMP280_SLAVE_ADDRESS 0x76

// Power management registers
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C

// Accelerometer and Gyroscope configuration registers
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG 0x1B
#define INT_PIN_CFG 0x37

// Accelerometer data registers
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

// Gyroscope data registers
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

// Magnetometer control register
#define AK8963_CNTL1 0x0A
#define CONFIG 0x1A
#define SMPLRT_DIV 0x19

// Magnetometer data registers
#define AK8963_XOUT_L 0x03
#define AK8963_XOUT_H 0x04
#define AK8963_YOUT_L 0x05
#define AK8963_YOUT_H 0x06
#define AK8963_ZOUT_L 0x07
#define AK8963_ZOUT_H 0x08

// BMP280 register
#define BMP_Ctrl 0xF4

// Pressuer data register
#define PRESS_MSB 0xF7
#define PRESS_LSB 0xF8
#define PRESS_XLSB 0xF9

// Temperture data register
#define TEMP_MSB 0xFA
#define TEMP_LSB 0xFB
#define TEMP_XLSB 0xFC

extern int variable[20];

#endif
