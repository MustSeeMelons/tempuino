// MLX90614ESF-BCC Infrared Sensor
// SSD1306 Driver 128X32 OLED
// https://www.electronicwings.com/arduino/arduino-i2c

#include <stdint.h>
#include <Wire.h>
#include "ssd1306.h"
#include "mlx.h"

TwoWire *ourWire = &Wire;

uint8_t OLED_ADDR = 0x3c;

void setup()
{
  Serial.begin(9600);

  ssd1306_128x32_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  mlx_init();
}

void loop()
{
  ssd1306_fillScreen(0x00);

  double ambientTemp = mlx_get_temp(MLX_TA);
  double objectTemp = mlx_get_temp(MLX_OBJ_1);

  const uint8_t ambient_buffer[32] = {};

  // Must use the special function, sprintf can't format floats
  char a_float_arr[6] = {};
  dtostrf(ambientTemp, 5, 2, a_float_arr);

  sprintf((char *)ambient_buffer, "Ambient: %sC, ", a_float_arr);

  const uint8_t object_buffer[32] = {};
  char o_float_arr[6] = {};
  dtostrf(objectTemp, 5, 2, o_float_arr);

  sprintf((char *)object_buffer, "Object: %sC, ", o_float_arr);

  ssd1306_printFixed(0, 8, (char *)ambient_buffer, STYLE_BOLD);
  ssd1306_printFixed(0, 16, (char *)object_buffer, STYLE_BOLD);

  delay(1000);
}