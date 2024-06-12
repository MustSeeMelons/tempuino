#include <stdint.h>
#include <Wire.h>
#include "ssd1306.h"
#include "mlx.h"
#include "Bounce2.h"
#include <stdbool.h>
#include "elapsedMillis.h"
// Some types for deeper library functions
#include "intf/ssd1306_interface.h"

// SSD1306 Driver 128X32 OLED
static const uint8_t d_width = 128;
static const uint8_t d_height = 32;

// References to display objects, for accessing some better functions
extern ssd1306_lcd_t ssd1306_lcd;
extern ssd1306_interface_t ssd1306_intf;

TwoWire *ourWire = &Wire;

static uint8_t action_btn_pin = 8;
static bool is_intro = true;
static Bounce bounce = Bounce();
static elapsedMillis idle_stamp;
static uint16_t idle_time = 30000; // ms

// Each byte is 8 vertical pixels
static uint8_t canvas[d_width][32 >> 3];

// Fill/unfill vars
static uint8_t anim_size = 8;
static uint8_t x_index = 0;
static uint8_t y_index = 0;
static bool is_fill = true;

// Arrow vars
static uint8_t idx = 0;        // Where do we start?
static uint8_t arrow_size = 8; // Same in h & w
static uint8_t arrow_scale = 4;
static bool is_forward = true;
static bool arrow_data[8][8] = {
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
};

void set_canvas_pixel(uint8_t x, uint8_t y)
{
  uint8_t p = y >> 3;  // Figure out which block it is
  uint8_t idx = y % 8; // Get bit index

  uint8_t value = 1 << idx; // Shuffle bit where we need it

  canvas[x][p] |= value; // Or bit into the new position
}

void clear_canvas_pixel(uint8_t x, uint8_t y)
{
  uint8_t p = y >> 3;  // Figure out which block it is
  uint8_t idx = y % 8; // Get bit index

  uint8_t value = ~(1 << idx); // Shuffle bit where we need it, and invert the result

  canvas[x][p] &= value; // And the abcent bit to clear
}

void setup()
{

  ssd1306_128x32_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  mlx_init();

  bounce.attach(action_btn_pin, INPUT_PULLUP);
  bounce.interval(50);
}

void tick_fill_anim()
{
  // Set current pixels
  for (uint8_t i = x_index; i < (x_index + anim_size); i++)
  {
    for (uint8_t j = y_index; j < (y_index + anim_size); j++)
    {
      if (is_fill)
      {
        set_canvas_pixel(i, j);
      }
      else
      {
        clear_canvas_pixel(i, j);
      }
    }
  }

  // Increment indexes
  x_index += anim_size;

  if (x_index == d_width)
  {
    x_index = 0;
    y_index += anim_size;
  }

  if (y_index == d_height)
  {
    y_index = 0;
    x_index = 0;

    is_fill = !is_fill;
  }
}

void tick_arrow_anim()
{
  // Clear old arrow
  for (uint8_t x = 0; x < arrow_size; x++)
  {
    for (uint8_t y = 0; y < arrow_size; y++)
    {
      for (uint8_t i = x * arrow_scale; i < x * arrow_scale + arrow_scale; i++)
      {
        for (uint8_t j = y * arrow_scale; j < y * arrow_scale + arrow_scale; j++)
        {
          if (is_forward)
          {
            clear_canvas_pixel(i + idx - 1, j);
          }
          else
          {
            clear_canvas_pixel(i + idx + 1, j);
          }
        }
      }
    }
  }

  // Switch arrow direction once we have traveled enough
  if (idx >= d_width * 0.1)
  {
    is_forward = false;
  }
  else if (idx == 0)
  {
    is_forward = true;
  }

  // Fill in the new arrow
  for (uint8_t x = 0; x < arrow_size; x++)
  {
    for (uint8_t y = 0; y < arrow_size; y++)
    {
      if (arrow_data[y][x] == true)
      {
        for (uint8_t i = x * arrow_scale; i < x * arrow_scale + arrow_scale; i++)
        {
          for (uint8_t j = y * arrow_scale; j < y * arrow_scale + arrow_scale; j++)
          {
            set_canvas_pixel(i + idx, j);
          }
        }
      }
    }
  }

  // XXX some interpolation with bounces would be nice, but at 8Mhz it is just too slow
  if (is_forward)
  {
    idx++;
  }
  else
  {
    idx--;
  }
}

/**
 * @brief Sends over the current state of the canvas to the screen.
 *
 */
void paint_lcd()
{
  ssd1306_lcd.set_block(0, 0, d_width);

  // Must iterate horizontaly as page flips vertically
  for (uint8_t y = 0; y < d_height >> 3; y++)
  {

    for (uint8_t x = 0; x < d_width; x++)
    {
      ssd1306_lcd.send_pixels1(canvas[x][y]);
    }

    ssd1306_lcd.next_page();
  }

  ssd1306_intf.stop();
}

void loop()
{

  if (is_intro)
  {
    tick_arrow_anim();
    paint_lcd();
  }

  bounce.update();

  if (bounce.read() == LOW)
  {
    idle_stamp = 0;

    if (is_intro)
    {
      is_intro = false;
    }
    // XXX we are not using ambient

    double ambientTemp = mlx_get_temp(MLX_TA);
    double objectTemp = mlx_get_temp(MLX_OBJ_1);

    // Must use the special function, sprintf can't format floats
    const uint8_t ambient_buffer[32] = {};
    char a_float_arr[6] = {};
    dtostrf(ambientTemp, 5, 2, a_float_arr);

    sprintf((char *)ambient_buffer, "Ambient: %sC, ", a_float_arr);

    const uint8_t object_buffer[32] = {};
    char o_float_arr[6] = {};
    dtostrf(objectTemp, 5, 2, o_float_arr);

    sprintf((char *)object_buffer, "%sC ", o_float_arr);

    ssd1306_fillScreen(0x00);
    ssd1306_printFixedN(0, 8, (char *)object_buffer, EFontStyle::STYLE_BOLD, EFontSize::FONT_SIZE_2X);

    delay(100);
  }

  // Show the animation after a while
  if (!is_intro && idle_stamp > idle_time)
  {
    is_intro = true;

    // Reset animation vars if any

    // Fill
    x_index = 0;
    y_index = 0;
    is_fill = true;

    // Arrow
    idx = 0; // Where do we start?
    is_forward = true;
  }
}