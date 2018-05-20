#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_SIZE 		4096
#define TABLE_ENTRY		1024
#define KERNEL_START	0x400000
#define VID_PTE			0xB8
#define VID_VIRTUAL		34
#define FOURMB			0x400000

uint32_t page_dir[TABLE_ENTRY] __attribute__((aligned (PAGE_SIZE))); // Create page directory, aligned to 4kb
uint32_t page_table[TABLE_ENTRY] __attribute__((aligned (PAGE_SIZE))); // Create page table, aligned to 4kb
uint32_t vid_pt[TABLE_ENTRY] __attribute__((aligned (PAGE_SIZE))); // Create page table, aligned to 4kb

void init_paging(); //map initializes the page directory, page table, enabling paging functionality,map virtual memory to physical memory
void add_process(int32_t process_num);
uint8_t* vid_paging(int32_t process_num);
void remapping(int32_t process_num);

#endif
