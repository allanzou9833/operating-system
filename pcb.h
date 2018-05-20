#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "syscall.h"

#define MAX_FILES 8

#define STDIN 0
#define STDOUT 1

#define EIGHTMB	0x800000
#define EIGHTKB	0x2000

typedef struct pcb_block{
	optable_t optable; //optable for operations
	uint32_t inode;	//inode number
	uint32_t file_pos;	//position of the file in the block
	uint32_t status;	//being used or not
} pcb_block_t;

typedef struct pcb pcb_t;
struct pcb{
	pcb_block_t files[MAX_FILES];	//pcb can hold 8 files total including stdin stdout
	uint32_t parent_process;	//parent process number
	uint32_t parent_esp;	//stack pointer
	uint32_t parent_ebp;	//base pointer 
	uint32_t process_number;	//the process 
	uint8_t user_buffer[128];
	//int32_t dir_address;
	//uint32_t store_entry_point[4];

};

void pcb_init(pcb_t* pcb);
pcb_t* get_pcb();
pcb_t* pcb_address(int32_t process);
#endif
