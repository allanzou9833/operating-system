#include "keyboard.h"
#include "lib.h"
#include "types.h"
#include "terminal_driver.h"
#include "syscall.h"
#include "paging.h"
#include "x86_desc.h"

int32_t terminal_open(const uint8_t* filename){
	return 0;
}

int32_t terminal_close(int32_t fd){
	return -1;
}

/* terminal read
 * 
 * Read keyboard input from the terminal
 * Inputs: None
 * Outputs: number of bytes read 
 * Side Effects: write keyboard input to buffer
 * Coverage: terminal read
 * Files: terminal_driver.h/c
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	sti();
	while(!enter_flag);
	// int i;
	// old_enter = enter_indic;
	// enter_indic = curr_loc;
	// if (enter_indic-old_enter > BUFFER_SIZE-1){
	// 	for (i = old_enter;i< old_enter+BUFFER_SIZE-1;i++){
	// 		terminals[curr_terminal].input_buf[i-old_enter] = ((uint8_t*)buf)[i];
	// 	}
	// 	old_enter = curr_loc;
	// 	enter_indic = curr_loc;
	// }else{
	// 	for (i = old_enter;i< enter_indic;i++){
	// 		terminals[curr_terminal].input_buf[i-old_enter] = ((uint8_t*)buf)[i];
	// 	}
	// }
	
	// return enter_indic-old_enter;
	enter_flag = 0;
	int i = 0;

	int bytes_read = 0;
	while(terminals[curr_terminal].input_buf[i] != '\n' && i < BUFFER_SIZE){
		((uint8_t*)buf)[i] = terminals[curr_terminal].input_buf[i];
		terminals[curr_terminal].input_buf[i] = '\0';
		i++; bytes_read++;
	}
	((uint8_t*)buf)[i] = terminals[curr_terminal].input_buf[i];
	bytes_read++;
	kbd_buf_loc = 0;
	return bytes_read;
}
/* terminal write
 * 
 * Write from buffer to screen
 * Inputs: None
 * Outputs: number of bytes write 
 * Side Effects: write keyboard input to screen
 * Coverage: terminal write
 * Files: terminal_driver.h/c
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	int i;
	int k;
	int j;
	for (k = 0; k < nbytes; k++){
		if (curr_loc > SCREEN_WIDTH*SCREEN_HEIGHT-1){
			for (i = 0; i < SCREEN_HEIGHT-1; i++){
				for (j = 0; j<SCREEN_WIDTH;j++){
					video_mem[i*SCREEN_WIDTH+j] = video_mem[(i+1)*SCREEN_WIDTH+j];
				}
			}
			for (j = 0; j<(SCREEN_WIDTH/2);j++){
					video_mem[(SCREEN_HEIGHT-1)*SCREEN_WIDTH+j*2] = ' ';
					video_mem[(SCREEN_HEIGHT-1)*SCREEN_WIDTH+j*2+1] = ATTRIB;
			}
			curr_loc = (SCREEN_HEIGHT-1)*SCREEN_WIDTH;
		}
		if(((uint8_t*)buf)[k] == '\n') {
			curr_loc += SCREEN_WIDTH - (curr_loc % SCREEN_WIDTH);
			continue;
		}
		if(backspace_flag){
			backspace_flag = 0;
			video_mem[--curr_loc] = ATTRIB;
			video_mem[--curr_loc] = ((uint8_t*)buf)[k];
		}
		else{
			video_mem[curr_loc++] = ((uint8_t*)buf)[k];
			video_mem[curr_loc++] = ATTRIB;
		}
	}
	update_cursor(curr_loc);
	return nbytes;
}

/* 
  * terminal_init()
  *	  DESCRIPTION: initialization for multiple terminals
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: creates multiple terminals at will
  */

void terminal_init(){
	int i, j;
	for(i = 0; i < 3; i++){	//3 terminals max for our code. This is all initialization
		terminals[i].status = CLOSED; 
		terminals[i].num_processes = 0;
		terminals[i].in_buf_loc = 0;
		terminals[i].screen_mem_loc = 0;
		for(j = 0; j < SCREEN_WIDTH*SCREEN_HEIGHT;){
			terminals[i].screen_mem[j++] = '\0';
			terminals[i].screen_mem[j++] = ATTRIB;
		}
	}
	terminals[TERMINAL1].status = OPEN;	//open up terminal 1
	curr_terminal = TERMINAL1;
}

/* 
  * terminal_switch(uint32_t terminal_num)
  *	  DESCRIPTION: switches terminals 
  *	  INPUT: terminal number to switch to 
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: switches terminal 
  */

void terminal_switch(uint32_t terminal_num){
	//Save current terminals buffer locations
	terminals[curr_terminal].in_buf_loc = kbd_buf_loc;
	terminals[curr_terminal].screen_mem_loc = curr_loc;

	//Save video memory to terminal screen memory
	memcpy(terminals[curr_terminal].screen_mem, video_mem, SCREEN_SIZE);

	asm volatile(
		"movl %%esp, %0;"
		"movl %%ebp, %1;"
		:"=r"(terminals[curr_terminal].esp), "=r"(terminals[curr_terminal].ebp)
		:		
	);
	terminals[curr_terminal].esp0 = tss.esp0;

	//Load buffer locations of terminal being switched to
	kbd_buf_loc = terminals[terminal_num].in_buf_loc;
	curr_loc = terminals[terminal_num].screen_mem_loc;
	update_cursor(curr_loc);

	//Load terminals screen memory to video memory
	memcpy(video_mem, terminals[terminal_num].screen_mem, SCREEN_SIZE);

	//Change current process and terminal
	curr_process = terminals[terminal_num].curr_process;
	curr_terminal = terminal_num;
	//Context switch
	if(terminals[terminal_num].status == CLOSED){
		terminals[terminal_num].status = OPEN;
		execute((uint8_t*)"shell ");
	}
	else{
		add_process(curr_process);
		tss.esp0 = terminals[curr_terminal].esp0;
		remapping(curr_process);
		//Load esp, ebp, cr3
		asm volatile(
			"movl %0, %%esp;"
			"movl %1, %%ebp;"
			:
			:"r"(terminals[curr_terminal].esp), "r"(terminals[curr_terminal].ebp)
		);
	}
}
