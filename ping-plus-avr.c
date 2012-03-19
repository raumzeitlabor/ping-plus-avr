#include <avr/io.h>
#include "usbdrv.h"
#include "ping-plus-avr.h"
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>

uint8_t pixmap[(COLS*ROWS) / 8];
//uint8_t writepixmap[(COLS*ROWS) / 8];

static uchar buffer[64];
static uchar currentPosition, bytesRemaining;

USB_PUBLIC usbMsgLen_t  usbFunctionSetup(uchar setupData[8]) {
	usbRequest_t *rq = (void *)setupData;   // cast to structured data for parsing

	return 0;
	switch(rq->bRequest){
	    case 1:
	    	currentPosition = 0;                // initialize position index

	    	bytesRemaining = rq->wLength.word;  // store the amount of data requested
			if(bytesRemaining > sizeof(buffer)) // limit to buffer size
				bytesRemaining = sizeof(buffer);
			return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
	    }

	return 0;
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
    uchar i;
    return 0;
    if(len > bytesRemaining)                // if this is the last incomplete chunk
        len = bytesRemaining;               // limit to the amount we can store
    bytesRemaining -= len;
    for(i = 0; i < len; i++)
        buffer[currentPosition++] = data[i];
    return bytesRemaining == 0;             // return 1 if we have all data
}


void setpixel (uint8_t x, uint8_t y) {
	pixmap[(y*COLS)+(x/8)] |= (1 << (x %8));
}

void clearpixel (uint8_t x, uint8_t y) {
	pixmap[(y*COLS)+(x/8)] &= (1 << (x %8));
}

uint8_t getpixel (uint8_t x, uint8_t y) {
	return pixmap[(y*COLS)+(x/8)] & (1 << (x %8));
}

void build_screen (void) {
	int x, y;
	for (y=0;y<ROWS;y++) {
		for (x=0;x<COLS;x++) {
			if (getpixel(x,y)) {
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
	int x;
	int y;
	int xpos = 0, ypos=0;
	// Setup ports
	DDRC |= (1 << LINE0) | (1 << LINE1) | (1 << LINE2) | (1 << LINE3) | (1 << LINE4) | (1 << LINE5);
	DDRB |= (1 << LINE6) | (1 << CLOCK);
	DDRD |= (1 << DATA);

	PORTC |= (1 << LINE0) | (1 << LINE1) | (1 << LINE2) | (1 << LINE3) | (1 << LINE4) | (1 << LINE5);
	PORTB |= (1 << LINE6);

	for (y=0;y<7;y++) {
		for (x=0;x<90;x++) {
			setpixel(x,y);
		}
	}

	wdt_enable(WDTO_2S);

	usbInit();
	usbDeviceDisconnect();
	_delay_ms(100);
	usbDeviceConnect();


	sei();

	flash_line(0);

	while (1) {
		wdt_reset();
		build_screen();

		for (y=0;y<ROWS;y++) {
			for (x=0;x<COLS;x++) {
				clearpixel(x,y);
			}
		}

		setpixel(xpos, ypos);
		xpos++;

		if (xpos == VISIBLECOLS) {
			xpos = 0;
			ypos++;
		}

		if (ypos == ROWS) {
			ypos = 0;
		}


		_delay_ms(20);

		//usbPoll();
	}


}



void shift () {
    PORTB &= ~(1<<CLOCK);
    PORTB |= (1<<CLOCK);
}

void data_high () {
	PORTD |= (1<<DATA);
}

void data_low () {
	PORTD &= ~(1<<DATA);
}

void flash_line (int i) {
	switch (i) {
		case 0:
			PORTC &= ~(1<<LINE0);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE0);
			break;
		case 1:
			PORTC &= ~(1<<LINE1);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE1);
			break;
		case 2:
			PORTC &= ~(1<<LINE2);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE2);
			break;
		case 3:
			PORTC &= ~(1<<LINE3);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE3);
			break;
		case 4:
			PORTC &= ~(1<<LINE4);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE4);
			break;
		case 5:
			PORTC &= ~(1<<LINE5);
			_delay_us(LINE_FLASH_US);
			PORTC |= (1<<LINE5);
			break;
		case 6:
			PORTB &= ~(1<<LINE6);
			_delay_us(LINE_FLASH_US);
			PORTB |= (1<<LINE6);
			break;
	}
}
