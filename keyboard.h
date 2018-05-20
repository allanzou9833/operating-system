#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KBD_EOI		0x1
#define KBD_STATUS	0x64
#define KBD_DATA	0x60
#define ENTER 		0x1C
#define CAPS		0x3A
#define LEFTSHIFTPRESSED	0x2A
#define RIGHTSHIFTPRESSED	0x36
#define LEFTSHIFTRELEASED	0xAA
#define RIGHTSHIFTRELEASED	0xB6
#define CTRLPRESSED			0x1D
#define CTRLRELEASED		0x9D
#define ALTPRESSED			0x38	
#define ALTRELEASED			0xB8	
#define BACKSPACE			0x0E
#define F1	0x3B
#define F2	0x3C
#define F3	0x3D

uint8_t shift_pressed;
int8_t caps_toggle;
uint8_t ctrl_pressed;
uint8_t enter_flag;
uint8_t backspace_flag;
uint8_t alt_pressed;

uint32_t kbd_buf_loc;

void keyboard_handler();
void init_keyboard();




#endif
