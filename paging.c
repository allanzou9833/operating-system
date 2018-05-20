#include "paging.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"

/*init_paging():																		*/
/*	INPUT: None 																		*/
/*	INPUT: None																			*/
/*	RETURN VALUE: None																	*/
/*	SIDE EFFECT: initializes the page directory, page table,							*/
/*				 enabling paging functionality,map virtual memory to physical memory	*/
/*Description: 																			*/
/*This function initializes the page directory, page table,enabling paging functionality,*/
/*Enabling 4Mib page functionality														 */
void init_paging(){
	unsigned int i;
	for (i = 0; i < TABLE_ENTRY; i++){ // set all entries except first and second to not present, 8mb to 4gb
		page_dir[i] = 0x00000002;		/* R/W */
		page_table[i] = 0x00000002;		/* R/W */
	}

	page_table[VID_PTE] = VIDEO | 0x3;		/* R/W, Present */
	page_dir[0] = (uint32_t)page_table | 0x1;		/* Present */
	page_dir[1] = KERNEL_START | 0x83;	/* Page Size = 4MiB, R/W, Present */

	/* Turn on paging */
	asm volatile ( 
		"movl %0, %%eax;"		/*Init eax register*/
		"movl %%eax, %%cr3;"	/*Get value from cr3 register*/
		:
		:"r"(page_dir)			/*Load page_dir into cr3 */
		:"%eax");
	asm volatile (
		"movl %%cr4, %%eax;"	/*Enabling 4Mib page */
		"orl $0x00000010, %%eax;"
		"movl %%eax, %%cr4;"	/*Set cr4 register, 8th bits to 1*/
		:
		:
		:"%eax");
	asm volatile (
		"movl %%cr0, %%eax;"	/*Enabling paging functionality */
		"orl $0x80000000, %%eax;" /*Set 32th bits in CR0 to 1 */
		"movl %%eax, %%cr0;"
		:
		:
		:"%eax");
}

/* void add_process(int32_t process_num);
 * description: adds a process 
 * Inputs: process number
 * Return Value: none
 * Side effects: adds a page for the process
 */

void add_process(int32_t process_num){
	uint32_t address = KERNEL_START + process_num*FOURMB;
	page_dir[/*31+process_num*/32] = address | 0x87; /*4MB page, R/W, User, Present */
	asm volatile(
		"movl %%cr3, %%eax;"
		"movl %%eax, %%cr3;"
		:
		:
		: "%eax");
}

/* uint8_t* vid_paging(int32_t process_num);
 * description: paging for video memory 
 * Inputs: process number
 * Return Value: address of the page 
 * Side effects: adds a page for the process
 */

uint8_t* vid_paging(int32_t process_num){
	vid_pt[0] = VIDEO | 0x7;
	page_dir[VID_VIRTUAL+process_num] = (uint32_t)vid_pt | 0x7;
	asm volatile ( 
		"movl %0, %%eax;"		/*Init eax register*/
		"movl %%eax, %%cr3;"	/*Get value from cr3 register*/
		:
		:"r"(page_dir)			/*Load page_dir into cr3 */
		:"%eax");
	return (uint8_t*)((VID_VIRTUAL+process_num)*FOURMB);
}

/* void remapping(int32_t process_num);
 * description: same thing as vid_paging with no return value 
 * Inputs: process number
 * Return Value: none 
 * Side effects: adds a page for the process
 */

void remapping(int32_t process_num){
	vid_pt[0] = VIDEO | 0x7;
	page_dir[VID_VIRTUAL+process_num] = (uint32_t)vid_pt | 0x7;
	asm volatile ( 
		"movl %0, %%eax;"		/*Init eax register*/
		"movl %%eax, %%cr3;"	/*Get value from cr3 register*/
		:
		:"r"(page_dir)			/*Load page_dir into cr3 */
		:"%eax");
}


