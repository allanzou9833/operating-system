#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminal_driver.h"

/* Key releases being read for 1, 2, q ; 7 outputing even though key release not read*/
/* 1 - Y
 * 2 - Z
 * q - down arrow
 * 7 - random
 * hacky solution - only write to vidmem if keycode is within scan_code index
 */
 
 /* 
  * keyboard_handler()
  *	  DESCRIPTION: IRQ handler for keyboard interrupts. Check keyboard status and 
  *				   then read in keycode from keyboard. Make sure keycode is valid
  *				   before writing to video memory.
  *	  INPUT: none
  *   OUTPUT: keycode mapped character to video memory at current location
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: none
  */
  
 
/* Mapping of keycode -> character */
static uint8_t scan_code[59] = 
{
	'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
	'\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0'
};
/* Mapping of keycode -> shift_pressed_character */
static uint8_t scan_code_shift[59] = 
{
	'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
	'\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'
};
/* Mapping of keycode -> capslock_enabled_character */
static uint8_t scan_code_caps[59] = 
{
	'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
	'\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', '\0',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'
};
 /* 
  * keyboard_handler()
  *	  DESCRIPTION: Handel keyboard interrupts	   
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: append character to buffer, clear screen, delete character in buffer
  */
void keyboard_handler()
{
	cli();
	uint8_t status;
	uint8_t keycode;
	
	status = inb(KBD_STATUS); //check keyboard status
	if(status & 0x01){

		keycode = inb(KBD_DATA); //get input keycode from keyboard

		if (keycode == CAPS) //if capsLock is enabled, negate flag
			caps_toggle = -caps_toggle;
		else if (keycode == LEFTSHIFTPRESSED || keycode == RIGHTSHIFTPRESSED) //if shift is pressed, set flag
			shift_pressed = 1;
		else if (keycode == LEFTSHIFTRELEASED || keycode == RIGHTSHIFTRELEASED) //if shift is released, set flag
			shift_pressed = 0;
		else if (keycode == CTRLPRESSED) //if ctrl is pressed, set flag
			ctrl_pressed = 1;
		else if (keycode == CTRLRELEASED) //if ctrl is released, set flag
			ctrl_pressed = 0;
		else if (keycode == ALTPRESSED)
			alt_pressed = 1;
		else if (keycode == ALTRELEASED)
			alt_pressed = 0;
		else if(keycode == ENTER){
			enter_flag = 1;
			//curr_loc += SCREEN_WIDTH - (curr_loc % SCREEN_WIDTH);
		}
		else if(keycode == BACKSPACE && video_mem[curr_loc-4] != '>'){ //backspace delete characters in buffer
			terminals[curr_terminal].input_buf[--kbd_buf_loc] = ' ';
			backspace_flag = 1;;
			terminal_write(1, &terminals[curr_terminal].input_buf[kbd_buf_loc], 1);
		}

		if(kbd_buf_loc >= BUFFER_SIZE){
			send_eoi(KBD_EOI);
			sti();
			return;
		}

		if(keycode < 59){
			if(scan_code[keycode] == 'l' && ctrl_pressed == 1){ // clear screen, set cursor to position 0
				curr_loc = 0;
				clear();
				kbd_buf_loc = 0; //reset indicators
			}
			else if(scan_code[keycode] != NULL && caps_toggle == -1 && shift_pressed == 0){ // not capitalized/shift character
				/* video_mem[kbd_buf_loc++] = scan_code[keycode];
				video_mem[kbd_buf_loc++] = ATTRIB; */
				terminals[curr_terminal].input_buf[kbd_buf_loc++] = scan_code[keycode]; //load into keyboard buffer
				terminal_write(1, &terminals[curr_terminal].input_buf[kbd_buf_loc-1], 1);
			}
			else if(scan_code_shift[keycode] != NULL && shift_pressed == 1){ //shift character
				/* video_mem[kbd_buf_loc++] = scan_code_shift[keycode];
				video_mem[kbd_buf_loc++] = ATTRIB; */
				terminals[curr_terminal].input_buf[kbd_buf_loc++] = scan_code_shift[keycode]; //load into keyboard buffer
				terminal_write(1, &terminals[curr_terminal].input_buf[kbd_buf_loc-1], 1);
			}
			else if(scan_code_caps[keycode] != NULL && caps_toggle == 1){ // capitalized character
					/* video_mem[kbd_buf_loc++] = scan_code_caps[keycode];
					video_mem[kbd_buf_loc++] = ATTRIB; */
					terminals[curr_terminal].input_buf[kbd_buf_loc++] = scan_code_caps[keycode]; //load into keyboard buffer
					terminal_write(1, &terminals[curr_terminal].input_buf[kbd_buf_loc-1], 1);
			}
		}
		else if(keycode == F1 && alt_pressed == 1){
			send_eoi(KBD_EOI);
			sti();
			terminal_switch(TERMINAL1);
		}
		else if(keycode == F2 && alt_pressed == 1){
			send_eoi(KBD_EOI);
			sti();
			terminal_switch(TERMINAL2);
		}
		else if(keycode == F3 && alt_pressed == 1){
			send_eoi(KBD_EOI);
			sti();
			terminal_switch(TERMINAL3);
		}
	}
	send_eoi(KBD_EOI);
	sti();
}
	
 /* 
  * init_keyboard()
  *	  DESCRIPTION: Initialize keyboard interrutps by enabling IRQ1 			   
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: IRQ1 on master PIC enabled
  */
void init_keyboard()
{
	kbd_buf_loc = 0; //initialize indicator and flags
	caps_toggle = -1;
	shift_pressed = 0;
	ctrl_pressed = 0;
	enter_flag = 0;
	backspace_flag = 0;
	alt_pressed = 0;
	enable_irq(1); //enable keyboard
}
