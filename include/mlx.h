#include "Arduino.h"
#include <Wire.h>

/*
    MLX90614ESF-BCC Infrared Sensor
    10 - 100 KHz
*/

// RAM addresses
#define MLX_IR1 0x04   // Raw data channel 1
#define MLX_IR2 0x05   // Raw data channel 2
#define MLX_TA 0x06    // Ambient (sensor) temprature
#define MLX_OBJ_1 0x07 // Object temprature
#define MLX_OBJ_2 0x08 // XXX not sure, seems to be -50 ob OBJ1

// EEPROM addresses - not used, used for configuration of PWM and other nick nacks

void mlx_init();

float mlx_get_temp(uint8_t temp_reg_address);
