#ifndef HD44780_H_
#define HD44780_H_

#include <stdint.h>
#include <avr/pgmspace.h>

// instruction register bit positions
#define HD44780_CLR             0      // DB0: clear display
#define HD44780_HOME            1      // DB1: return to home position
#define HD44780_ENTRY_MODE      2      // DB2: set entry mode
#define HD44780_ENTRY_INC       1      //   DB1: 1=increment, 0=decrement
#define HD44780_ENTRY_SHIFT     0      //   DB2: 1=display shift on
#define HD44780_ON              3      // DB3: turn lcd/cursor on
#define HD44780_ON_DISPLAY      2      //   DB2: turn display on
#define HD44780_ON_CURSOR       1      //   DB1: turn cursor on
#define HD44780_ON_BLINK        0      //     DB0: blinking cursor ?
#define HD44780_MOVE            4      // DB4: move cursor/display
#define HD44780_MOVE_DISP       3      //   DB3: move display (0-> cursor) ?
#define HD44780_MOVE_RIGHT      2      //   DB2: move right (0-> left) ?
#define HD44780_FUNCTION        5      // DB5: function set
#define HD44780_FUNCTION_8BIT   4      //   DB4: set 8BIT mode (0->4BIT mode)
#define HD44780_FUNCTION_2LINES 3      //   DB3: two lines (0->one line)
#define HD44780_FUNCTION_10DOTS 2      //   DB2: 5x10 font (0->5x7 font)
#define HD44780_CGRAM           6      // DB6: set CG RAM address
#define HD44780_DDRAM           7      // DB7: set DD RAM address
#define HD44780_BUSY            7      // DB7: LCD is busy

// set entry mode: display shift on/off, dec/inc cursor move direction
#define HD44780_ENTRY_DEC            0x04   // display shift off, dec cursor move dir
#define HD44780_ENTRY_DEC_SHIFT      0x05   // display shift on,  dec cursor move dir
#define HD44780_ENTRY_INC_           0x06   // display shift off, inc cursor move dir
#define HD44780_ENTRY_INC_SHIFT      0x07   // display shift on,  inc cursor move dir

// display on/off, cursor on/off, blinking char at cursor position
#define HD44780_DISP_OFF             0x08   // display off
#define HD44780_DISP_ON              0x0C   // display on, cursor off
#define HD44780_DISP_ON_BLINK        0x0D   // display on, cursor off, blink char
#define HD44780_DISP_ON_CURSOR       0x0E   // display on, cursor on
#define HD44780_DISP_ON_CURSOR_BLINK 0x0F   // display on, cursor on, blink char

// move cursor/shift display
#define HD44780_MOVE_CURSOR_LEFT     0x10   // move cursor left  (decrement)
#define HD44780_MOVE_CURSOR_RIGHT    0x14   // move cursor right (increment)
#define HD44780_MOVE_DISP_LEFT       0x18   // shift display left
#define HD44780_MOVE_DISP_RIGHT      0x1C   // shift display right

// function set: set interface data length and number of display lines
#define HD44780_FUNCTION_4BIT_1LINE  0x20   // 4-bit interface, single line, 5x7 dots
#define HD44780_FUNCTION_4BIT_2LINES 0x28   // 4-bit interface, dual line,   5x7 dots
#define HD44780_FUNCTION_8BIT_1LINE  0x30   // 8-bit interface, single line, 5x7 dots 0b00110000
#define HD44780_FUNCTION_8BIT_2LINES 0x38   // 8-bit interface, dual line,   5x7 dots


namespace hd44780
{


void init (uint8_t attribute = HD44780_DISP_ON);
void command (uint8_t command);
void goto_xy (uint8_t x, uint8_t y);
void put (char c);
void puts (const char *s);
void puts_p (const char *progmem_str);
inline void clear ();
void puti (int i, int radix = 10);

}

#endif // HD44780_H_
