// MLX90614ESF-BCC Infrared Sensor
// SSD1306 Driver 128X32 OLED
// https://www.electronicwings.com/arduino/arduino-i2c

#include <stdint.h>
#include "ssd1306.h"
#include <Adafruit_MLX90614.h>
#include <Wire.h>

uint8_t OLED_ADDR = 0x3c;
uint8_t TEMP_ADDR = 0x5a;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// TODO Remove adafruit lib, create own
// TODO Go over oled lib, possible create own
// TODO Don't use wire?

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  ssd1306_128x32_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  mlx.begin();
}

void perform_temp_read()
{
  // T01 RAM addr 0x07 will sweep between 0x27AD to 0x7fff;
  Wire.beginTransmission(TEMP_ADDR);
  Wire.endTransmission();
}

void loop()
{
  ssd1306_fillScreen(0x00);

  double ambientTemp = mlx.readAmbientTempC();
  double objectTemp = mlx.readObjectTempC();

  const uint8_t ambient_buffer[32] = {};

  sprintf((char *)ambient_buffer, "Ambient: %fC, ", ambientTemp);

  const uint8_t object_buffer[32] = {};
  sprintf((char *)object_buffer, "Object: %fC, ", objectTemp);

  ssd1306_printFixed(0, 8, (char *)ambient_buffer, STYLE_BOLD);
  ssd1306_printFixed(0, 16, (char *)object_buffer, STYLE_BOLD);

  delay(1000);
}