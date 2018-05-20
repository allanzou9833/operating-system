/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
static uint8_t master_mask = 0xff; /* IRQs 0-7  */
static uint8_t slave_mask = 0xff;  /* IRQs 8-15 */

/*
 * i8259_init(void);
 * Description: Initializes the 8259 PIC and masks all interrupts
 * Inputs: None
 * Outputs: None
 * Return values: None
 * Side effects: None
 */
void i8259_init(void) {
	
	/* Master PIC initialization */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT2); 
	outb(ICW3_MASTER, MASTER_8259_PORT2);
	outb(ICW4, MASTER_8259_PORT2);
	
	/* Slave PIC initialization*/
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT2);
	outb(ICW3_SLAVE, SLAVE_8259_PORT2);
	outb(ICW4, SLAVE_8259_PORT2);
	
	/* mask all interrupts */
	outb(master_mask, MASTER_8259_PORT2);
	outb(slave_mask, SLAVE_8259_PORT2);
}


/*
 * enable_irq(uint32_t irq_num);
 * Description: Enables a certain IRQ by changing master and slave mask
 * Inputs: IRQ number
 * Outputs: None
 * Return values: None
 * Side effects: Changes master and slave masks
 */
void enable_irq(uint32_t irq_num) {

	//if IRQ is for master
	if(irq_num < NUM_IRQ_LINES)
		master_mask &= ~(1 << irq_num);
	//if IRQ is for slave
	else{
		slave_mask &= ~(1 << (irq_num-NUM_IRQ_LINES));
		enable_irq(SLAVE_TO_MASTER);
	}
	//mask ports
	outb(master_mask, MASTER_8259_PORT2);
	outb(slave_mask, SLAVE_8259_PORT2);
}

/*
 * disable_irq(uint32_t irq_num);
 * Description: Disables a certain IRQ by changing master and slave mask
 * Inputs: IRQ number
 * Outputs: None
 * Return values: None
 * Side effects: Changes master and slave masks
 */
void disable_irq(uint32_t irq_num) {
	
	//if IRQ is for master
	if(irq_num < NUM_IRQ_LINES)
		master_mask |= 1 << irq_num;
	//if IRQ is for slave
	else
		slave_mask |= 1 << (irq_num - NUM_IRQ_LINES);
	//mask ports
	outb(master_mask, MASTER_8259_PORT2);
	outb(slave_mask, SLAVE_8259_PORT2);
}

/*
 * send_eoi(uint32_t irq_num);
 * Description: Sends EOI after every IRQ
 * Inputs: IRQ number
 * Outputs: None
 * Return values: None
 * Side effects: None
 */
void send_eoi(uint32_t irq_num) {
	//check if IRQ is for slave
	if(irq_num >= NUM_IRQ_LINES){
		//send EOI
		outb(EOI|(irq_num - NUM_IRQ_LINES), SLAVE_8259_PORT);
		send_eoi(SLAVE_TO_MASTER);
	}	
	//send EOI
	outb(EOI|irq_num, MASTER_8259_PORT);
}
