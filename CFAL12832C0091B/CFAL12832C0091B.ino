//===========================================================================
//
//  Code written for Seeeduino v4.2 runnung at 3.3v
//
//  CRYSTALFONTZ CFAL12832C-0091Bx 128x23 OLED in SPI mode
//
//  ref: https://www.crystalfontz.com/product/cfal12832c0091bw
//       https://www.crystalfontz.com/product/cfal12832c0091by
//
//  2018 - 02 - 24 Brent A. Crosby
//  2019 - 08 - 06 Trevin Jorgenson
//  2022 - 06 - 15 Kelsey Zaches
//===========================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//===========================================================================
//
// Breakout board and wires may be useful:
//   https://www.crystalfontz.com/product/cfa10054
//   https://www.crystalfontz.com/product/wrjmpy41
//   https://www.crystalfontz.com/product/wrjmpy40
//
// The controller is a Solomon Systech SSD1306
//   https://www.crystalfontz.com/controllers/SolomonSystech/SSD1306/
//
//============================================================================
#include <avr/io.h>
#include <SPI.h>
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src\SPI.cpp
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src\SPI.h
#include <avr/pgmspace.h>
#include "bitmaps.h"
//============================================================================
//
// The CFAL12832C-0091B is a 3.3v device. You need a 3.3v Arduino or the
// Seeeduino v4.2 set to 3.3v to operate this code properly:
// http://www.seeedstudio.com/item_detail.html?p_id=2517
//
//============================================================================
// LCD SPI & control lines
//   ARD   | Port | OLED                     | Wire       
//---------+------+--------------------------+------------
//  3.3V   |      | 3.3v VDD                 | Orange
//  GND    |      | Ground                   | Black
// --------+------+--------------------------+------------
//  #8/D8  |  PB0 | LCD_DC   Data/Command    | Yellow
//  #9/D9  |  PB1 | LCD_RES  Reset           | Purple
// #10/D10 |  PB2 | LCD_CS   chip select     | Grey
// #11/D11 |  PB3 | LCD_D1   MOSI            | Green 
// #12/D12 |  PB4 | not used (would be MISO) | 
// #13/D13 |  PB5 | LCD_D0   SCK clock       | White
//============================================================================
#define CLR_DC    (PORTB &= ~(0x01))
#define SET_DC    (PORTB |=  (0x01))
#define CLR_RST   (PORTB &= ~(0x02))
#define SET_RST   (PORTB |=  (0x02))
#define CLR_CS    (PORTB &= ~(0x04))
#define SET_CS    (PORTB |=  (0x04))
#define CLR_MOSI  (PORTB &= ~(0x08))
#define SET_MOSI  (PORTB |=  (0x08))
#define CLR_SCK   (PORTB &= ~(0x20))
#define SET_SCK   (PORTB |=  (0x20))
//============================================================================
#define NUMBER_OF_SCREENS (3)

const SCREEN_IMAGE *const screens[NUMBER_OF_SCREENS] PROGMEM=
  {
  &SPI_Logo,
  &I2C_Logo,
  &Batt_Volume
  };


//============================================================================
void sendcommand(uint8_t command)
{
  // Select the LCD's command register
  CLR_DC;
  // Select the LCD controller
  CLR_CS;
  //Send the command via SPI:
  SPI.transfer(command);
  // Deselect the LCD controller
  SET_CS;
}
//============================================================================
void senddata(uint8_t data)
{
  // Select the LCD's data register
  SET_DC;
  // Select the LCD controller
  CLR_CS;
  //Send the command via SPI:
  SPI.transfer(data);
  // Deselect the LCD controller
  SET_CS;
}
//============================================================================
// Straight up code: ~1.3mS to update
void show_128_x_4_bitmap(const SCREEN_IMAGE *OLED_image)
{
  uint8_t
    column;
  uint8_t
    row;

  for (row = 0; row < 4; row++)
  {
    sendcommand(0x00);      //lower column address
    sendcommand(0x10);      //upper column address
    sendcommand(0xB0 + row);  //set page address
    for (column = 0; column < 128; column++)
    {
      //Read this byte from the program memory / flash
      senddata(pgm_read_byte( &(OLED_image  ->bitmap_data[row][column]) ));
    }
  }
}
//============================================================================
//With an 8MHz SPI clock, this function takes 617uS
//128*4*8 =4096 bits in 617uS = 6.638 megabits per second
// void show_128_x_4_bitmap_faster(const SCREEN_IMAGE OLED_image)
// {
//   uint8_t
//     column;
//   uint8_t
//     row;
//   uint8_t
//     this_byte;

//   for (row = 0; row < 4; row++)
//   {
//     sendcommand(0x00);      //lower column address
//     sendcommand(0x10);      //upper column address
//     sendcommand(0xB0 + row);  //set page address
//     // Select the LCD's data register
//     SET_DC;
//     // Select the LCD controller
//     CLR_CS;
//     //Use a while loop, faster than a for.
//     column = 0;
//     //Get the first flash access out of the way.
//     this_byte = pgm_read_byte(&OLED_image[row * 128 + column]);
//     do
//     {
//       //Start shifting this byte
//       SPDR = this_byte;
//       //Do all the other things we can while this byte is shifting out the port.
//       column++;
//       this_byte = pgm_read_byte(&OLED_image[row * 128 + column]);
//       //Additional delay needed (7 nops works, 6 fails)
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//       __asm__("nop\n\t");
//     } while (column <= 127);
//     //Wait for the last transmission to complete before
//     //raising CS.
//     while (!(SPSR & _BV(SPIF))); // wait
//     // Deselect the LCD controller
//     SET_CS;
//   }
// }
//============================================================================
//Updated code, 2016-04-14
#define SSD1306B_DCDC_CONFIG_PREFIX_8D          (0x8D)
#define SSD1306B_DCDC_CONFIG_7p5v_14            (0x14)
#define SSD1306B_DCDC_CONFIG_6p0v_15            (0x15)
#define SSD1306B_DCDC_CONFIG_8p5v_94            (0x94)
#define SSD1306B_DCDC_CONFIG_9p0v_95            (0x95)
#define SSD1306B_DISPLAY_OFF_YES_SLEEP_AE       (0xAE)
#define SSD1306B_DISPLAY_ON_NO_SLEEP_AF         (0xAF)
#define SSD1306B_CLOCK_DIVIDE_PREFIX_D5         (0xD5)
#define SSD1306B_MULTIPLEX_RATIO_PREFIX_A8      (0xA8)
#define SSD1306B_DISPLAY_OFFSET_PREFIX_D3       (0xD3)
#define SSD1306B_DISPLAY_START_LINE_40          (0x40)
#define SSD1306B_SEG0_IS_COL_0_A0               (0xA0)
#define SSD1306B_SEG0_IS_COL_127_A1             (0xA1)
#define SSD1306B_SCAN_DIR_UP_C0                 (0xC0)
#define SSD1306B_SCAN_DIR_DOWN_C8               (0xC8)
#define SSD1306B_COM_CONFIG_PREFIX_DA           (0xDA)
#define SSD1306B_COM_CONFIG_SEQUENTIAL_LEFT_02  (0x02)
#define SSD1306B_COM_CONFIG_ALTERNATE_LEFT_12   (0x12)
#define SSD1306B_COM_CONFIG_SEQUENTIAL_RIGHT_22 (0x22)
#define SSD1306B_COM_CONFIG_ALTERNATE_RIGHT_32  (0x32)
#define SSD1306B_CONTRAST_PREFIX_81             (0x81)
#define SSD1306B_PRECHARGE_PERIOD_PREFIX_D9     (0xD9)
#define SSD1306B_VCOMH_DESELECT_PREFIX_DB       (0xDB)
#define SSD1306B_VCOMH_DESELECT_0p65xVCC_00     (0x00)
#define SSD1306B_VCOMH_DESELECT_0p71xVCC_10     (0x10)
#define SSD1306B_VCOMH_DESELECT_0p77xVCC_20     (0x20)
#define SSD1306B_VCOMH_DESELECT_0p83xVCC_30     (0x30)
#define SSD1306B_ENTIRE_DISPLAY_FORCE_ON_A5     (0xA5)
#define SSD1306B_ENTIRE_DISPLAY_NORMAL_A4       (0xA4)
#define SSD1306B_INVERSION_NORMAL_A6            (0xA6)
#define SSD1306B_INVERSION_INVERTED_A7          (0xA7)

void Initialize_CFAL12832C(void)
{
  //Thump the reset.  
  _delay_ms(1);
  CLR_RST;
  _delay_ms(1);
  SET_RST;
  _delay_ms(5);

  //Set the display to sleep mode for the rest of the init.
  sendcommand(SSD1306B_DISPLAY_OFF_YES_SLEEP_AE);

  //Set the clock speed, nominal ~105FPS
  //Low nibble is divide ratio
  //High level is oscillator frequency
  sendcommand(SSD1306B_CLOCK_DIVIDE_PREFIX_D5);
  sendcommand(0x80);//177Hz measured

  //Set the multiplex ratio to 1/32
  //Default is 0x3F (1/64 Duty), we need 0x1F (1/32 Duty)
  sendcommand(SSD1306B_MULTIPLEX_RATIO_PREFIX_A8);
  sendcommand(0x1F);

  //Set the display offset to 0 (default)
  sendcommand(SSD1306B_DISPLAY_OFFSET_PREFIX_D3);
  sendcommand(0x00);

  //Set the display RAM display start line to 0 (default)
  //Bits 0-5 can be set to 0-63 with a bitwise or
  sendcommand(SSD1306B_DISPLAY_START_LINE_40);

  //Enable DC/DC converter, 7.5v
  sendcommand(SSD1306B_DCDC_CONFIG_PREFIX_8D);
  sendcommand(SSD1306B_DCDC_CONFIG_7p5v_14);

  //Map the columns correctly for our OLED glass layout
  sendcommand(SSD1306B_SEG0_IS_COL_127_A1);

  //Set COM output scan correctly for our OLED glass layout
  sendcommand(SSD1306B_SCAN_DIR_UP_C0);

  //Set COM pins correctly for our OLED glass layout
  sendcommand(SSD1306B_COM_CONFIG_PREFIX_DA);
  sendcommand(SSD1306B_COM_CONFIG_SEQUENTIAL_LEFT_02);

  //Set Contrast Control / SEG Output Current / Iref
  sendcommand(SSD1306B_CONTRAST_PREFIX_81);
  sendcommand(0xBF);  //magic # from factory

  //Set precharge (low nibble) / discharge (high nibble) timing
  //precharge = 1 clock
  //discharge = 15 clocks
  sendcommand(SSD1306B_PRECHARGE_PERIOD_PREFIX_D9); //Set Pre-Charge period        
  sendcommand(0xF1);

  //Set VCOM Deselect Level
  sendcommand(SSD1306B_VCOMH_DESELECT_PREFIX_DB);
  sendcommand(SSD1306B_VCOMH_DESELECT_0p83xVCC_30);

  //Make sure Entire Display On is disabled (default)
  sendcommand(SSD1306B_ENTIRE_DISPLAY_NORMAL_A4);

  //Make sure display is not inverted (default)
  sendcommand(SSD1306B_INVERSION_NORMAL_A6);

  //Get out of sleep mode, into normal operation
  sendcommand(SSD1306B_DISPLAY_ON_NO_SLEEP_AF);
}
//============================================================================
uint8_t
current_screen;
void setup(void)
{
  //Set up port B
  DDRB = 0x2F;

  //Drive the ports to a reasonable starting state.
  SET_RST;
  SET_DC;
  SET_CS;
  CLR_MOSI;
  CLR_SCK;

  // Initialize SPI. By default the clock is 4MHz. The chip is good to 10 MHz
  SPI.begin();
  //Bump the clock to 8MHz. Appears to be the maximum.
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  //Fire up the OLED
  Initialize_CFAL12832C();

  current_screen = 0;
}
//============================================================================
void  loop(void)
{
    uint8_t
    current_screen;
  //Put up some bitmaps from flash
  for(current_screen=0;current_screen<NUMBER_OF_SCREENS;current_screen++)
    {
    show_128_x_4_bitmap((SCREEN_IMAGE *)pgm_read_word(&screens[current_screen]));
    //Wait a bit . . .
    _delay_ms(2000);
    }

}
//============================================================================

