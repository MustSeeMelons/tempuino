#include "Arduino.h"
#include "stdbool.h"
#include "stdint.h"
#include "mlx.h"

// RAM addresses
#define MLX_IR1 0x04   // Raw data channel 1
#define MLX_IR2 0x05   // Raw data channel 2
#define MLX_TA 0x06    // Ambient (sensor) temprature
#define MLX_OBJ_1 0x07 // Object temprature
#define MLX_OBJ_2 0x08 // XXX not sure, seems to be -50 ob OBJ1

extern TwoWire *ourWire;

// EEPROM addresses - not used, used for configuration of PWM and other nick nacks

static uint8_t MLX_ADDR = 0x5a;
static uint8_t MLX_RES_BYTE_COUNT = 3;

void mlx_init()
{
    ourWire->begin();
}

/**
 * @brief Response is always 16 bits of data followed by 8 bits of PEC.
 * The PEC includes all bits except the START, REPEATED START, STOP, ACK, and NACK bits.
 * The PEC is a CRC-8 with polynomial X8+X2+X1+1
 *
 * @param reg_address
 * @param output - must have a length of atleast 3!
 */
static void sensor_fetch_data(uint8_t reg_address, uint8_t *output)
{
    uint8_t buffer_tx[1] = {reg_address};

    // Send over the register we wish to read
    ourWire->beginTransmission(MLX_ADDR);
    ourWire->write(buffer_tx, 1);
    ourWire->endTransmission(false);

    // Blockingly Read three bytes
    size_t recv = ourWire->requestFrom(MLX_ADDR, MLX_RES_BYTE_COUNT, 0, 0, 1);

    // If we don't get out bytes, simply return
    if (recv != MLX_RES_BYTE_COUNT)
    {
        return;
    }

    // Obtain bytes and read them
    for (uint8_t i = 0; i < MLX_RES_BYTE_COUNT; i++)
    {
        output[i] = ourWire->read();
    }
}

/**
 * @brief Read temprature from a register in a blocking fashion.
 *
 * @param temp_reg_address
 * @return `float` temprature in glorious C, not F
 */
float mlx_get_temp(uint8_t temp_reg_address)
{
    uint8_t buffer_rx[3];
    sensor_fetch_data(temp_reg_address, buffer_rx);

    float temp = buffer_rx[0] | uint16_t(buffer_rx[1]) << 8;
    temp *= 0.02;   // To Kelvin
    temp -= 273.15; // To Celcius

    return temp;
}
