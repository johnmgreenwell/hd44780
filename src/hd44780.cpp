#include "hd44780.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that it's in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

namespace PeripheralIO
{

HD44780::HD44780(hd44780_lcd_mode mode, const uint8_t* data_pins, uint8_t rs, uint8_t enable)
: _data_pins(data_pins, (uint8_t)mode)
, _rs_pin(rs)
, _rw_pin(255)
, _enable_pin(enable)
{
  _rw_active = false;
  init((uint8_t)mode, data_pins, rs, 255, enable);
}

HD44780::HD44780(hd44780_lcd_mode mode, const uint8_t* data_pins, uint8_t rs, uint8_t rw, uint8_t enable)
: _data_pins(data_pins, (uint8_t)mode)
, _rs_pin(rs)
, _rw_pin(rw)
, _enable_pin(enable)
{
  _rw_active = true;
  init((uint8_t)mode, data_pins, rs, rw, enable);
}

// HD44780::HD44780(uint8_t mode, uint8_t* four_data_pins, uint8_t rs, uint8_t rw, uint8_t enable)
// : _data_pins(four_data_pins, 4)
// , _rs_pin(rs)
// , _rw_pin(rw)
// , _enable_pin(enable)
// {
//   init(1, four_data_pins, rs, rw, enable);
// }

// HD44780::HD44780(uint8_t mode, uint8_t* four_data_pins, uint8_t rs, uint8_t enable)
// : _data_pins(four_data_pins, 4)
// , _rs_pin(rs)
// , _rw_pin(255)
// , _enable_pin(enable)
// {
//   init(1, four_data_pins, rs, 255, enable);
// }

void HD44780::init(uint8_t mode, const uint8_t* data_pins, uint8_t rs, uint8_t rw, uint8_t enable)
{
//   _rs_pin = rs;
//   _rw_pin = rw;
//   _enable_pin = enable;
  
//   _data_pins[0] = d0;
//   _data_pins[1] = d1;
//   _data_pins[2] = d2;
//   _data_pins[3] = d3; 
//   _data_pins[4] = d4;
//   _data_pins[5] = d5;
//   _data_pins[6] = d6;
//   _data_pins[7] = d7; 

  if (mode == (uint8_t)LCD_8BITMODE)
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  begin(16, 1);
}

void HD44780::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  _rs_pin.pinMode(GPIO_OUTPUT);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (_rw_active) { 
    _rw_pin.pinMode(GPIO_OUTPUT);
  }
  _enable_pin.pinMode(GPIO_OUTPUT);
  
  // Do these once, instead of every time a character is drawn for speed reasons.
  for (int i=0; i<((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i)
  {
    _data_pins.pinMode(i, GPIO_OUTPUT);
  } 

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40 ms after power rises above 2.7 V
  // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
  HAL::delay_us(50000); 
  // Now we pull both RS and R/W low to begin commands
  _rs_pin.digitalWrite(false);
  _enable_pin.digitalWrite(false);
  if (_rw_active) { 
    _rw_pin.digitalWrite(false);
  }
  
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the Hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    HAL::delay_us(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    HAL::delay_us(4500); // wait min 4.1ms
    
    // third go!
    write4bits(0x03); 
    HAL::delay_us(150);

    // finally, set to 4-bit interface
    write4bits(0x02); 
  } else {
    // this is according to the Hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    HAL::delay_us(4500);  // wait more than 4.1 ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    HAL::delay_us(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

void HD44780::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void HD44780::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  HAL::delay_us(2000);  // this command takes a long time!
}

void HD44780::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  HAL::delay_us(2000);  // this command takes a long time!
}

void HD44780::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/ 0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/ 0
  }
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void HD44780::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void HD44780::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void HD44780::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void HD44780::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void HD44780::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void HD44780::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void HD44780::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void HD44780::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void HD44780::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void HD44780::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void HD44780::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void HD44780::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void HD44780::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void HD44780::command(uint8_t value) {
  send(value, false);
}

inline size_t HD44780::write(uint8_t value) {
  send(value, true);
  return 1; // assume success
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void HD44780::send(uint8_t value, uint8_t mode) {
  _rs_pin.digitalWrite(mode);

  // if there is a RW pin indicated, set it low to Write
  if (_rw_active) { 
    _rw_pin.digitalWrite(false);
  }
  
  if (_displayfunction & LCD_8BITMODE) {
    write8bits(value); 
  } else {
    write4bits(value>>4);
    write4bits(value);
  }
}

void HD44780::pulseEnable(void) {
  _enable_pin.digitalWrite(false);
  HAL::delay_us(1);    
  _enable_pin.digitalWrite(true);
  HAL::delay_us(1);    // enable pulse must be >450 ns
  _enable_pin.digitalWrite(false);
  HAL::delay_us(100);  // commands need >37 us to settle
}

void HD44780::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    _data_pins.digitalWrite(i, (value >> i) & 0x01);
  }

  pulseEnable();
}

void HD44780::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    _data_pins.digitalWrite(i, (value >> i) & 0x01);
  }
  
  pulseEnable();
}

}

// EOF
