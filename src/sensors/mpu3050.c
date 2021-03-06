/*

FF32lite from FocusFlight, a new alternative firmware
for the Naze32 controller

Original work Copyright (c) 2013 John Ihlein

This file is part of FF32lite.

Includes code and/or ideas from:

  1)BaseFlight
  2)S.O.H. Madgwick

FF32lite is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FF32lite is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FF32lite. If not, see <http://www.gnu.org/licenses/>.

*/

///////////////////////////////////////////////////////////////////////////////

#include "board.h"

///////////////////////////////////////////////////////////////////////////////
// MPU3050 Defines and Variables
///////////////////////////////////////////////////////////////////////////////

// Address

#define MPU3050_ADDRESS 0x68

// Registers

#define MPU3050_X_OFFS_H      0x0C
#define MPU3050_X_OFFS_L      0x0D
#define MPU3050_Y_OFFS_H      0x0E
#define MPU3050_Y_OFFS_L      0x0F
#define MPU3050_Z_OFFS_H      0x10
#define MPU3050_Z_OFFS_L      0x11
#define MPU3050_SMPLRT_DIV    0x15
#define MPU3050_DLPF_FS_SYNC  0x16
#define MPU3050_INT_CFG       0x17
#define MPU3050_TEMP_OUT      0x1B
#define MPU3050_GYRO_OUT      0x1D
#define MPU3050_USER_CTRL     0x3D
#define MPU3050_PWR_MGM       0x3E

// Bits

#define FS_SEL_2000_DPS       0x18

#define ACTL                  0x00
#define OPEN                  0x00
#define LATCH_INT_EN          0x20
#define INT_ANYRD_2CLEAR      0x10
#define RAW_RDY_EN            0x01

#define H_RESET               0x80
#define INTERNAL_OSC          0x00

///////////////////////////////////////

#define LOW_PASS_FILTER 0x18    // 256 Hz Low pass filter, 8 kHz internal sample rate
//#define LOW_PASS_FILTER 0x19  // 188 Hz Low pass filter, 1 kHz internal sample rate
//#define LOW_PASS_FILTER 0x1A  //  98 Hz Low pass filter, 1 kHz internal sample rate
//#define LOW_PASS_FILTER 0x1B  //  42 Hz Low pass filter, 1 kHz internal sample rate
//#define LOW_PASS_FILTER 0x1C  //  20 Hz Low pass filter, 1 kHz internal sample rate
//#define LOW_PASS_FILTER 0x1D  //  10 Hz Low pass filter, 1 kHz internal sample rate
//#define LOW_PASS_FILTER 0x1E  //   5 Hz Low pass filter, 1 kHz internal sample rate

#if (LOW_PASS_FILTER == 0x18)
#define SAMPLE_RATE_DIVISOR 0x07        // 1000 Hz = 8000/(7 + 1)
#else
#define SAMPLE_RATE_DIVISOR 0x00        // 1000 Hz = 1000/(0 + 1)
#endif

///////////////////////////////////////////////////////////////////////////////
// Read MPU3050
///////////////////////////////////////////////////////////////////////////////

void readMpu3050(void)
{
    uint8_t I2C2_Buffer_Rx[8];

    // Get data from device
    i2cRead(I2C2, MPU3050_ADDRESS, MPU3050_TEMP_OUT, 8, I2C2_Buffer_Rx);

    rawMpuTemperature.bytes[1] = I2C2_Buffer_Rx[0];
    rawMpuTemperature.bytes[0] = I2C2_Buffer_Rx[1];

    rawGyro[ROLL ].bytes[1] = I2C2_Buffer_Rx[2];
    rawGyro[ROLL ].bytes[0] = I2C2_Buffer_Rx[3];
    rawGyro[PITCH].bytes[1] = I2C2_Buffer_Rx[4];
    rawGyro[PITCH].bytes[0] = I2C2_Buffer_Rx[5];
    rawGyro[YAW  ].bytes[1] = I2C2_Buffer_Rx[6];
    rawGyro[YAW  ].bytes[0] = I2C2_Buffer_Rx[7];
}

///////////////////////////////////////////////////////////////////////////////
// Compute MPU3050 Temperature Compensation Bias
///////////////////////////////////////////////////////////////////////////////

void computeMpu3050TCBias(void)
{
    mpuTemperature = (float) (rawMpuTemperature.value + 13200) / 280.0f + 35.0f;

    gyroTCBias[ROLL ] = eepromConfig.gyroTCBiasSlope[ROLL ] * mpuTemperature + eepromConfig.gyroTCBiasIntercept[ROLL ];
    gyroTCBias[PITCH] = eepromConfig.gyroTCBiasSlope[PITCH] * mpuTemperature + eepromConfig.gyroTCBiasIntercept[PITCH];
    gyroTCBias[YAW  ] = eepromConfig.gyroTCBiasSlope[YAW  ] * mpuTemperature + eepromConfig.gyroTCBiasIntercept[YAW  ];
}

///////////////////////////////////////////////////////////////////////////////
// Compute MPU3050 Runtime Bias
///////////////////////////////////////////////////////////////////////////////

void computeMpu3050RTBias(void)
{
    uint8_t axis;
    uint16_t samples;
    float gyroSum[3] = { 0.0f, 0.0f, 0.0f };

    mpuCalibrating = true;

    for (samples = 0; samples < 2000; samples++)
    {
        readMpu3050();

        computeMpu3050TCBias();

        gyroSum[ROLL] += rawGyro[ROLL].value - gyroTCBias[ROLL];
        gyroSum[PITCH] += rawGyro[PITCH].value - gyroTCBias[PITCH];
        gyroSum[YAW] += rawGyro[YAW].value - gyroTCBias[YAW];

        delayMicroseconds(1000);
    }

    for (axis = ROLL; axis < 3; axis++) {
        gyroRTBias[axis] = (float) gyroSum[axis] / 2000.0f;

    }

    mpuCalibrating = false;
}

///////////////////////////////////////////////////////////////////////////////
// MPU3050 Initialization
///////////////////////////////////////////////////////////////////////////////

void initMpu3050(void)
{
    i2cWrite(I2C2, MPU3050_ADDRESS, MPU3050_PWR_MGM, H_RESET);
    i2cWrite(I2C2, MPU3050_ADDRESS, MPU3050_PWR_MGM, INTERNAL_OSC);
    i2cWrite(I2C2, MPU3050_ADDRESS, MPU3050_DLPF_FS_SYNC, LOW_PASS_FILTER | FS_SEL_2000_DPS);
    i2cWrite(I2C2, MPU3050_ADDRESS, MPU3050_SMPLRT_DIV, SAMPLE_RATE_DIVISOR);
    i2cWrite(I2C2, MPU3050_ADDRESS, MPU3050_INT_CFG, 0);

    delay(100);

    computeMpu3050RTBias();
}

///////////////////////////////////////////////////////////////////////////////
