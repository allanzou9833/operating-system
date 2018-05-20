#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

#define VIRTUAL_MEM		32
#define TASK_SHELL		0x800000
#define TASK_SECOND		0xC00000
#define TABLE_ENTRY		1024
#define PAGE_SIZE 		4096
#define IMAGE_MEM		0x08048000
#define MAX_PROCESS		6
#define PCB_OFFSET		0x400000
#define _128MB			0x08000000
#define _160MB			0x0a000000
uint32_t program_page_table[TABLE_ENTRY] __attribute__((aligned (PAGE_SIZE)));
int32_t curr_process;

typedef struct optable{	//optable for functions open close read write 
	int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} optable_t;

//syscalls 

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

//helper functions
int32_t find_open_process();

//jumptable
extern optable_t rtc_op;
extern optable_t file_op;
extern optable_t dir_op;
extern optable_t terminal_op;

#endif
