#include "syscall.h"
#include "types.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal_driver.h"
#include "pcb.h"
#include "paging.h"
#include "x86_desc.h"

optable_t rtc_op = {rtc_open, rtc_close, rtc_read, rtc_write};		//these are jump tables to which device or type of file we need to use 
optable_t file_op = {file_open, file_close, file_read, file_write};
optable_t dir_op = {dir_open, dir_close, dir_read, dir_write};
optable_t terminal_op = {terminal_open, terminal_close, terminal_read, terminal_write};

static int32_t total_num_processes = 0;	//global variable to keep track of number of processes 
static uint32_t processes[MAX_PROCESS] = {0, 0, 0, 0, 0, 0};
/* 
  * halt()
  *	  DESCRIPTION: halts the process 
  *	  INPUT: Status
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: halts the process
  */

int32_t halt(uint8_t status){
	//get current pcb
	pcb_t* my_PCB = get_pcb();

	total_num_processes--;
	terminals[curr_terminal].num_processes--;
	processes[curr_process-1] = 0;	//free up process number

	//close all files in file descriptor array
	int i;
	for (i = 0; i <MAX_FILES; i++){
		if(my_PCB->files[i].status == OPEN)
			close(i);
	}
	for (i = 0; i < 128; i++)
		my_PCB->user_buffer[i] = '\0';

	//Reboot shell if exiting first shell
	curr_process = my_PCB->parent_process;
	terminals[curr_terminal].curr_process = my_PCB->parent_process;
	if(my_PCB->process_number == my_PCB->parent_process)
		execute((uint8_t*)"shell");
	
	//Restore parent paging
	add_process(my_PCB->parent_process);
	tss.esp0 = my_PCB->parent_esp;

	uint32_t ret = (uint32_t)status;
	asm volatile (
	"movl %0, %%eax;"
	"movl %1, %%esp;"
	"movl %2, %%ebp;"
	"jmp EXECUTE_RETURN;"
	:
	:"r"(ret), "r"(my_PCB->parent_esp), "r"(my_PCB->parent_ebp)		
	:"%eax");
	return 0;
}

/* 
  * execute()
  *	  DESCRIPTION: executes the process 
  *	  INPUT: command
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: executes the process
  */

int32_t execute(const uint8_t* command){

	if(total_num_processes == MAX_PROCESS)
		return -1;
	//Parse Args
	uint8_t program[32];
	int i = 0;
	int j;
	for (j = 0; j < 32;j++){
		program[j] = '\0';
	}
	while(command[i] != ' ' && command[i] != '\0'){
		program[i] = command[i];
		i++;
	}

	//Check File Validity
	dentry_t prog_dir;
	uint8_t magic[4];
	if(read_dentry_by_name(program, &prog_dir) == -1)
		return -1;
	if(read_data(prog_dir.n_inode, 0, magic, 4) != 4)
		return -1;
	if(magic[0] != 0x7f || magic[1] != 0x45 || magic[2] != 0x4c || magic[3] != 0x46)
		return -1;

	//Program Paging
	total_num_processes++;
	terminals[curr_terminal].num_processes++;
	int32_t new_process;
	if(-1 == (new_process = find_open_process()))
		return -1;

	pcb_t* pcb = pcb_address(new_process);
	pcb_init(pcb);
	pcb->process_number = new_process;
	if(terminals[curr_terminal].num_processes == 1)
		pcb->parent_process = pcb->process_number;
	else{
		pcb_t* curr_pcb = get_pcb();
		pcb->parent_process = curr_pcb->process_number;
	}

	j = 0; 
	while(command[i] == ' ')
		i++;
	while(command[i] != '\0')
		pcb->user_buffer[j++] = command[i++];

	add_process(new_process);

	//Program Loader
	uint8_t entry_pt[4];
	if(read_data(prog_dir.n_inode, 24, entry_pt, 4) != 4)
		return -1;
	uint32_t entry_addr = entry_pt[0] | entry_pt[1]<<8 | entry_pt[2]<<16 | entry_pt[3]<<24;

	read_data(prog_dir.n_inode, 0, (uint8_t*)IMAGE_MEM, file_length(prog_dir.n_inode));

	terminals[curr_terminal].curr_process = new_process;
	curr_process = new_process;	//Update current running process

	//Save esp and ebp in pcb and current terminal
	asm volatile(
		"movl %%esp, %0;"
		"movl %%ebp, %1;"
		:"=r"(pcb->parent_esp), "=r"(pcb->parent_ebp)
		:
	);

	//Context Switch
	tss.ss0 = KERNEL_DS;
	tss.esp0 = EIGHTMB - EIGHTKB*(new_process) - 4;
	uint32_t process_stack = _128MB + FOURMB - 0x4; //128MB
	asm volatile (
		"movw %0, %%ax;"
		"movw %%ax, %%ds;"
		"pushl %0;"
		"pushl %1;"
		"pushfl;"
		"popl %%eax;"
		"orl $0x200, %%eax;"
		"pushl %%eax;"
		"pushl %2;"
		"pushl %3;"
		"iret;"
		"EXECUTE_RETURN:;"
		"leave;"
		"ret;"
		:
		:"g"(USER_DS), "g"(process_stack), "g"(USER_CS), "g"(entry_addr)
		:"eax"
	);

	return 0;
}

/* 
  * read()
  *	  DESCRIPTION: reads in the data  
  *	  INPUT: file descriptor, buffer, and number of bytes
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: reads data 
  */

int32_t read(int32_t fd, void* buf, int32_t nbytes){
	if(fd >= 8 || fd < 0)
		return -1;
	if(fd == 1)
		return -1;
	if(buf == NULL)
		return -1;

	pcb_t* read_pcb = get_pcb();	//first get pcb 

	if(read_pcb->files[fd].status == CLOSED)
		return -1;

	return read_pcb->files[fd].optable.read(fd, buf, nbytes);	//use jumptable to read whatever device/file 
}

/* 
  * write()
  *	  DESCRIPTION: writes in data  
  *	  INPUT: file descriptor, buffer, and number of bytes
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: writes in data using the jumptable
  */

int32_t write(int32_t fd, const void* buf, int32_t nbytes){
	if(fd >= 8 || fd < 1)
		return -1;
	if(buf == NULL)
		return -1;

	pcb_t* write_pcb = get_pcb();	//same thing as read, get pcb first
	
	if(write_pcb->files[fd].status == CLOSED)
		return -1;

	return write_pcb->files[fd].optable.write(fd, buf, nbytes);	//use jumptable to write 
}

/* 
  * open()
  *	  DESCRIPTION: initializes and opens whatever device/file the user wants to initialize 
  *	  INPUT: filename
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: initializes rtc or opens file or directory
  */

int32_t open(const uint8_t* filename){
	dentry_t open_dentry;
	
	if(read_dentry_by_name(filename, &open_dentry) == -1)	//if file does not exist, return -1;
		return -1;
	
	pcb_t* open_pcb = get_pcb();	//first get pcb
	int i;
	for(i = 2; i <MAX_FILES; i++) {
		if(open_pcb->files[i].status == CLOSED) {	//find the first open pcb block, and store data in there 
			open_pcb->files[i].status = OPEN;
			open_pcb->files[i].inode = open_dentry.n_inode;
			open_pcb->files[i].file_pos = 0;
			switch(open_dentry.ftype) {	//depending on type of file, switch
				case FTYPE_RTC:
				open_pcb->files[i].optable = rtc_op;
				break;
				case FTYPE_DIR:
				open_pcb->files[i].optable = dir_op;
				break;
				case FTYPE_FILE:
				open_pcb->files[i].optable = file_op;
				break;
				default:
				break;
			}		
			break;
		}	
	}
	
	if(i == MAX_FILES)	//if no open blocks, return -1
		return -1;	
	
	open_pcb->files[i].optable.open(filename);
	return i;
}

/* 
  * close()
  *	  DESCRIPTION: closes the file described and frees the pcb block  
  *	  INPUT: fd
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: opens up the block for future use 
  */

int32_t close(int32_t fd){
	if(fd == STDIN || fd == STDOUT)	//cannot close the first 2 blocks
		return -1;
	if(fd >= 8 || fd < 0)
		return -1;

	pcb_t* close_pcb = get_pcb();	//as always, get pcb
	
	if(close_pcb->files[fd].status == CLOSED)	//if there was nothing in there in the first place, return -1 
		return -1;
	
	close_pcb->files[fd].status = CLOSED;	//fd open for use 
	close_pcb->files[fd].file_pos = 0;
	
	return close_pcb->files[fd].optable.close(fd);
}

/* 
  * getargs()
  *	  DESCRIPTION: copies args from pcb to user-level buffer  
  *	  INPUT: buf, nbytes
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS:  
  */
int32_t getargs(uint8_t* buf, int32_t nbytes){
	if(buf == NULL)
		return -1;
	if(nbytes > 128)
		nbytes = 128;

	pcb_t* getargs_pcb = get_pcb();

	if(getargs_pcb->user_buffer[0] == '\0')
		return -1;
	memcpy(buf, getargs_pcb->user_buffer, nbytes);
	return 0;

}

/* 
  * vidmap(uint8_t** screen_start)
  *	  DESCRIPTION: maps the text-mode video memory into user space at a pre-set virtual address   
  *	  INPUT: screen_start
  *   OUTPUT: none
  *	  RETURN VALUE: -1 if fail, 0 if success
  *	  SIDE EFFECTS: calls vid_paging();
  */

int32_t vidmap(uint8_t** screen_start){
	if (screen_start == NULL){
		return -1;
	}
	if ((uint32_t)*screen_start > FOURMB){
		return -1;
	}
	pcb_t* vidmap_pcb = get_pcb();

	*screen_start = vid_paging(vidmap_pcb->process_number);

	return 0;
}

/*extra credit: not implemented*/

int32_t set_handler(int32_t signum, void* handler_address){
	return 0;
}

/*extra credit: not implemented*/

int32_t sigreturn(void){
	return 0;
}

/* 
  * find_open_process()
  *	  DESCRIPTION: finds the first open process    
  *	  INPUT: none
  *   OUTPUT: none
  *	  RETURN VALUE: the process number if success, -1 if fail
  *	  SIDE EFFECTS: none
  */

int32_t find_open_process(){
	int i;
	for(i = 0; i < MAX_PROCESS; i++){
		if(processes[i] == 0){
			processes[i] = 1;
			return i+1;
		}
	}
	return -1;
}
