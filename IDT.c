#include "x86_desc.h"
#include "lib.h"
#include "interrupts.h"

#define SYS_CALL 	0x80
#define KBD_INT		0x21
#define RTC_INT		0x28

/*
 * General exception handler
 * Description: Macro that makes a new function for a general exception
 * 				called once for each general exception below
 * Inputs: Name of exception and message to print out
 * Outputs: Message to print out on screen
 * Return values: None
 * Side effects: Clears screen and puts system in while(1) loop
 */

#define EXCEPTION(name, msg)	\
void name(){					\
	clear();					\
	printf("%s\n", msg);		\
	while(1);					\
}								\

/* Exception Handlers */
EXCEPTION(DE, "Divide Error");
EXCEPTION(DB, "Debug");
EXCEPTION(NMI, "Non-Maskable Interrupt");
EXCEPTION(BP, "Breakpoint");
EXCEPTION(OF, "Overflow");
EXCEPTION(BR, "BOUND Range Exceeded");
EXCEPTION(UD, "Invalid Opcode");
EXCEPTION(NM, "Device Not Available");
EXCEPTION(DF, "Double Fault");
EXCEPTION(CSO, "Coprocessor Segment Overrun");
EXCEPTION(TS, "Invalid TSS");
EXCEPTION(NP, "Segment Not Present");
EXCEPTION(SS, "Stack-Segment Fault");
EXCEPTION(GP, "General Protection");
EXCEPTION(PF, "Page Fault");
EXCEPTION(MF, "x87 Floating-Point Error");
EXCEPTION(AC, "Alignment Check");
EXCEPTION(MC, "Machine Check");
EXCEPTION(XF, "SIMD Floating-Point Exception");

/*
 * install_int_table()
 * Description: Initializes the IDT with the first 20 exceptions and other interrupts we need
 * Inputs: None
 * Outputs: None
 * Return values: None
 * Side effects: Allocates memory for the IDT
 */
void install_int_table()
{
	int i = 0;
	for(; i < NUM_VEC; ++i){
		//kernel level privilege
		idt[i].dpl = 0x0;
		idt[i].present = 0x1;
		idt[i].size = 0x1;
		idt[i].seg_selector = KERNEL_CS;
		
		/*Interrupt Gate (General Interrupt) */
		idt[i].reserved0 = 0x0;
		idt[i].reserved1 = 0x1;
		idt[i].reserved2 = 0x1;
		idt[i].reserved3 = 0x0;
		idt[i].reserved4 = 0x0;
		/* Trap Gate (General Exception) */
		//First 32 Intel defined exceptions
		if(i < 32)							
			idt[i].reserved3 = 0x1;
		
		if(i == SYS_CALL){
			//Syscalls also software interrupts
			idt[i].reserved3 = 0x1;
			//user level privilege
			idt[SYS_CALL].dpl = 0x3;
		}
	}
	SET_IDT_ENTRY(idt[1], DB);
	SET_IDT_ENTRY(idt[2], NMI);
	SET_IDT_ENTRY(idt[3], BP);
	SET_IDT_ENTRY(idt[4], OF);
	SET_IDT_ENTRY(idt[5], BR);
	SET_IDT_ENTRY(idt[6], UD);
	SET_IDT_ENTRY(idt[7], NM);
	SET_IDT_ENTRY(idt[8], DF);
	SET_IDT_ENTRY(idt[9], CSO);
	SET_IDT_ENTRY(idt[10], TS);
	SET_IDT_ENTRY(idt[11], NP);	
	SET_IDT_ENTRY(idt[12], SS);
	SET_IDT_ENTRY(idt[13], GP);
	SET_IDT_ENTRY(idt[14], PF);
	SET_IDT_ENTRY(idt[16], MF);
	SET_IDT_ENTRY(idt[17], AC);
	SET_IDT_ENTRY(idt[18], MC);
	SET_IDT_ENTRY(idt[19], XF);
	
	SET_IDT_ENTRY(idt[KBD_INT], keyboard_wrapper);
	
	SET_IDT_ENTRY(idt[RTC_INT], rtc_wrapper);

	SET_IDT_ENTRY(idt[SYS_CALL], syscall_wrapper);
	
	//load the IDT
	lidt(idt_desc_ptr);
}
