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
#endif
