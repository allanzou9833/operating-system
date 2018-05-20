#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H

#include "types.h"

#define BUFFER_SIZE 	128
#define SCREEN_WIDTH 	160
#define SCREEN_HEIGHT 	25
#define SCREEN_SIZE		SCREEN_WIDTH*SCREEN_HEIGHT
#define BLACK 			0x00
#define TERMINAL1		0
#define TERMINAL2		1
#define TERMINAL3		2

typedef struct terminal{
	int32_t status;
	int32_t num_processes;	//may not need this
	int32_t curr_process;	//current process # the terminal is running
	uint8_t screen_mem[SCREEN_SIZE];	//save video mem
	uint8_t input_buf[BUFFER_SIZE]; //each terminals terminal_buf
	uint32_t in_buf_loc;
	uint32_t screen_mem_loc;
	uint32_t esp0;
	uint32_t esp;
	uint32_t ebp;
	uint32_t cr3;
} terminal_t;

terminal_t terminals[3];

uint8_t terminal_buf[BUFFER_SIZE]; //terminal buffer has size of 128 bytes

/* curr location in video memory */
uint32_t curr_loc;
uint32_t curr_terminal;

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

void terminal_init();
void terminal_switch(uint32_t terminal_num);

#endif
