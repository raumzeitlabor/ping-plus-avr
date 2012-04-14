#include <avr/io.h>
#include "usbdrv.h"
#include "ping-plus-avr.h"
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "font.h"
#include <string.h>

uint8_t pixmap[(COLS * ROWS) / 8];

uchar controller = 0;

#define BUFFERLEN 256
#define VERSION_MAJOR 2
#define VERSION_MINOR 8
static int8_t SCROLLWAIT  = 4;
#define MODE_SCROLL 0
#define MODE_SCROLLOUT 1
//#define WAIT_DIR_CHANGE 100

static uint16_t WAIT_DIR_CHANGE = 100;

static uchar buffer[BUFFERLEN], offbuffer[BUFFERLEN];
static int16_t currentPosition;
static int16_t lineLength = 0; // Line length in pixels
static int16_t linePos = 0;
static uint8_t scrollWaitCounter = 0;
static uint8_t dir = 0;
static uint8_t dirChangeDelay = 0;
static uint8_t fadeOutPos = 0;
static uint8_t mode = 0;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *) data;
	static uchar replyBuf[2];

	usbMsgPtr = replyBuf;

	switch (rq->bRequest) {
	case 0:
		replyBuf[0] = rq->wValue.bytes[0];
		replyBuf[1] = rq->wValue.bytes[1];
		return 2;
		break;
	case 2:
		setpixel(rq->wValue.bytes[0], rq->wIndex.bytes[0]);
		return 0;
		break;
	case 3:
		clearpixel(rq->wValue.bytes[0], rq->wIndex.bytes[0]);
		return 0;
		break;
		// Put a char into the buffer
	case 4:
		if (currentPosition + 1 < BUFFERLEN) {
			offbuffer[currentPosition] = rq->wIndex.bytes[0];
			offbuffer[currentPosition + 1] = '\0';
			currentPosition++;
		}
		return 0;
		break;
		// Copies the off buffer to the main buffer, update line length
	case 5:
		copy_buffer();
		return 0;
		break;
		// Clears the char buffer
	case 10:
		clear_off_buffer();
		return 0;
		break;
	case 20:
		mode = MODE_SCROLLOUT;
		fadeOutPos = 0;
		break;
	case 25:
		WAIT_DIR_CHANGE = rq->wValue.bytes[0];
		break;
	case 26:
		SCROLLWAIT = rq->wValue.bytes[0];
		break;
	}
	return 0;
}

void clear_off_buffer () {
	memset(offbuffer, 0, BUFFERLEN);
	offbuffer[0] = '\0';
	currentPosition = 0;
	linePos = 0;
}

void copy_buffer () {
	uint8_t j;
	memcpy(buffer, offbuffer, BUFFERLEN);

	for (j = 0; j < BUFFERLEN; j++) {
		if (buffer[j] == '\0') {
			break;
		}
	}

	lineLength = j * 6;
	linePos = 0;
	mode = MODE_SCROLL;
}
void writeChar(int16_t x, int16_t y, unsigned char c) {
	uint8_t i,j,temp;

	for (i = 0; i < 5; i++) {
		if (c < 32) break;

		temp = pgm_read_byte(Font5x7 + (c - 0x20) * 5 + (i));

		for (j = 0; j < 7; j++) {
			if (CHECK_BIT(temp, j)) {
				setpixel(x + i, y + j);
			}
		}
	}
}

void setpixel(int16_t x, int16_t y) {
	if (x > VISIBLECOLS || x < 0 || y > ROWS || y < 0) {
		return;
	}
	pixmap[((y * COLS) + x) / 8] |= (1 << (x % 8));
}

void clearpixel(uint8_t x, uint8_t y) {
	if (x > VISIBLECOLS) {
		return;
	}
	if (y > ROWS) {
		return;
	}
	pixmap[((y * COLS) + x) / 8] &= ~(1 << (x % 8));
}

uint8_t getpixel(uint8_t x, uint8_t y) {
	return pixmap[((y * COLS) + x) / 8] & (1 << (x % 8));
}

void build_screen(void) {
	uint8_t x, y;
	for (y = 0; y < ROWS; y++) {
		for (x = 0; x < COLS; x++) {
			if (getpixel(x, (ROWS-1)-y)) {
				data_high();
			} else {
				data_low();
			}

			shift();
		}

		flash_line(y);
	}
}

int main(void) {
	int16_t j;
	uchar w;

	DDRC |= (1 << LINE0) | (1 << LINE1) | (1 << LINE2) | (1 << LINE3)
			| (1 << LINE4) | (1 << LINE5);
	DDRB |= (1 << LINE6) | (1 << CLOCK);
	DDRD |= (1 << DATA);

	PORTC |= (1 << LINE0) | (1 << LINE1) | (1 << LINE2) | (1 << LINE3)
			| (1 << LINE4) | (1 << LINE5);
	PORTB |= (1 << LINE6);

	wdt_enable(WDTO_2S);

	usbInit();
	usbDeviceDisconnect(); /* enforce re-enumeration, do this while interrupts are disabled! */
	w = 0;
	while (--w) { /* fake USB disconnect for > 250 ms */
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();

	flash_line(0);

	wdt_enable(WDTO_2S);

	memset(pixmap, 0, (COLS * ROWS) / 8);

	while (1) {
		usbPoll();

		switch (mode) {
		case MODE_SCROLL:


			for (j = (linePos/6); j < (linePos/6) + 18; j++) {
				if (buffer[j] == '\0') {
					break;
				}
				writeChar(j * 6 - linePos, 0, buffer[j]);
			}

			switch (dir) {
			case 0:
				if (linePos < lineLength - VISIBLECOLS) {
					if (scrollWaitCounter > SCROLLWAIT) {
						linePos++;
						scrollWaitCounter = 0;
					} else {
						scrollWaitCounter++;
					}
				} else {
					if (dirChangeDelay < WAIT_DIR_CHANGE) {
						dirChangeDelay++;
					} else {
						dirChangeDelay = 0;
						dir = 1;
					}

				}
				break;
			case 1:
				if (linePos > 0) {
					if (linePos > 2) {
						linePos-=2;
					} else {
						linePos = 0;
					}
				} else {
					if (dirChangeDelay < WAIT_DIR_CHANGE) {
						dirChangeDelay++;
					} else {
						dirChangeDelay = 0;
						dir = 0;
					}
				}
			}

			break;
			case MODE_SCROLLOUT:
				for (j = 0; j < BUFFERLEN; j++) {
					if (buffer[j] == '\0') {
						break;
					}
					writeChar(j * 6 - linePos, -fadeOutPos, buffer[j]);
				}

				if (scrollWaitCounter > SCROLLWAIT) {
					fadeOutPos++;
					scrollWaitCounter = 0;
				} else {
					scrollWaitCounter++;
				}

				if (fadeOutPos > 7) {
					mode = MODE_SCROLL;
					clear_off_buffer();
					copy_buffer();
				}

				break;
		}

		build_screen();

		memset(pixmap, 0, (COLS * ROWS) / 8);
		wdt_reset();
		usbPoll();
	}

}

void shift() {
	PORTB &= ~(1 << CLOCK);
	PORTB |= (1 << CLOCK);
}

void data_high() {
	PORTD |= (1 << DATA);
}

void data_low() {
	PORTD &= ~(1 << DATA);
}

void flash_line(int i) {

	switch (i) {
	case 0:
		PORTC &= ~(1 << LINE0);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE0);
		break;
	case 1:
		PORTC &= ~(1 << LINE1);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE1);
		break;
	case 2:
		PORTC &= ~(1 << LINE2);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE2);
		break;
	case 3:
		PORTC &= ~(1 << LINE3);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE3);
		break;
	case 4:
		PORTC &= ~(1 << LINE4);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE4);
		break;
	case 5:
		PORTC &= ~(1 << LINE5);
		_delay_us(LINE_FLASH_US);
		PORTC |= (1 << LINE5);
		break;
	case 6:
		PORTB &= ~(1 << LINE6);
		_delay_us(LINE_FLASH_US);
		PORTB |= (1 << LINE6);
		break;
	}
}
