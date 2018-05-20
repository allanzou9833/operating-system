#include "pcb.h"
#include "syscall.h"
#include "lib.h"

#define PCB_MASK 0xFFFFE000 //8KB = 2^13

/* 
  * PCB_init()
  *	  DESCRIPTION: Initialize PCB
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: initializes pcb by setting stdin and stdout
  */

void pcb_init(pcb_t* pcb){
	int i;
	for(i = 0; i < MAX_FILES; i++){
		pcb->files[i].inode = 0;
		pcb->files[i].file_pos = 0;
		pcb->files[i].status = CLOSED;
	}

	pcb->files[STDIN].optable = terminal_op;
	pcb->files[STDIN].status = OPEN;

	pcb->files[STDOUT].optable = terminal_op;
	pcb->files[STDOUT].status = OPEN;
}

/* 
  * get_pcb()
  *	  DESCRIPTION: returns the pcb of the process
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: the pcb of the process
  *	  SIDE EFFECTS: none
  */

pcb_t* get_pcb(){
	// pcb_t* pcb;
	// asm volatile (
	// 	"andl %%esp, %0;"
	// 	:"=r"(pcb)
	// 	:"r"(PCB_MASK)
	// );
	// return pcb;
	return pcb_address(curr_process);
}

/* 
  * pcb_address()
  *	  DESCRIPTION: returns the pcb address of the process
  *	  INPUT: process
  *   OUTPUT: none
  *	  RETURN VALUE: the pcb address of the process
  *	  SIDE EFFECTS: none
  */

pcb_t* pcb_address(int32_t process){
	return (pcb_t*)(EIGHTMB - process*EIGHTKB);
}
