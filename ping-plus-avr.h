#ifndef __pingplus_h_included__
#define __pingplus_h_included__

#define LINE0 PC0
#define LINE1 PC1
#define LINE2 PC2
#define LINE3 PC3
#define LINE4 PC4
#define LINE5 PC5
#define LINE6 PB1

#define CLOCK PB0
#define DATA  PD7

#define INPUT1 PD3
#define INPUT2 PD4
#define S0 PD6

#define LINE_FLASH_US 500

#define COLS 95
#define VISIBLECOLS 85
#define ROWS 7

void setpixel (int16_t x, int16_t y);
void clearpixel (uint8_t x, uint8_t y);
void copy_buffer (void);
void clear_off_buffer (void);
void writeChar (int16_t x, int16_t y, unsigned char c);
void flash_line (int i);
void data_low (void);
void data_high (void);
void shift (void);
void playsound (void);

const char MenuItem1[] PROGMEM = "Ja";
const char MenuItem2[] PROGMEM = "Nein";
const char MenuItem3[] PROGMEM = "Vielleicht";
const char MenuItem4[] PROGMEM = "Alle";
const char MenuItem5[] PROGMEM = "Keiner";
const char MenuItem6[] PROGMEM = "Diverse(s)";
const char MenuItem7[] PROGMEM = "Gleich";
const char MenuItem8[] PROGMEM = "Spaeter";
const char MenuItem9[] PROGMEM = "Niemals";
const char MenuItem10[] PROGMEM = "Pony";

const char * const MenuItemPointers[] PROGMEM =
{
		MenuItem1, MenuItem2, MenuItem3, MenuItem4, MenuItem5, MenuItem6,
				MenuItem7, MenuItem8, MenuItem9, MenuItem10
};


#define MAX_ITEMS 9
#endif
