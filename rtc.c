#include "rtc.h"

volatile int32_t read_flag; //a flag to see if the interrupt has been handled
/*
void set_rate(uint32_t freq)
Description: sets rate in correspondence to frequency that has been passed on 
input: frequency to set
output: none
return value: none
side effects: sets desired frequency 
*/

void set_rate(uint32_t freq) {
	uint32_t rate;

	//frequency = 32768 >> (rate - 1)
		
	if(freq == 2)
		rate = 15;
	else if(freq == 4)
		rate = 14;
	else if(freq == 8)
		rate = 13;
	else if(freq == 16)
		rate = 12;
	else if(freq == 32)
		rate = 11;
	else if(freq == 64)
		rate = 10;
	else if(freq == 128)
		rate = 9;
	else if(freq == 256)
		rate = 8;
	else if(freq == 512)
		rate = 7;
	else if(freq == 1024)
		rate = 6;
	
	
	rate &= 0x0F;			// rate is between 2 and 15
	cli();
	outb(REGISTER_A, CMOS_OUT);		//set index to register A and disable NMI	
	outb((freq & 0xF0) | rate, CMOS_IN); //write the rate into register A 
	sti();
}

/*
void init_rtc()
Description: initializes RTC
input: none
output: none 
return value: none
side effects: enables rtc interrupt 
*/
void init_rtc()
{
	enable_irq(RTC_IRQ_NUMB);  //rtc line is on the IRQ 8 (slave)
	
	uint8_t prev;  
	outb(REGISTER_B, CMOS_OUT); //disable NMI and select register B, 0x70/0x71 are IO ports of RTC and CMOS 
	prev = inb(CMOS_IN); //read current register value of B
	outb(REGISTER_B, CMOS_OUT); // set index again to reset the index to register D 
	outb(prev | BIT_6, CMOS_IN); //turn on bit 6 of B
	set_rate(2);
	read_flag = 0;
}

/*
void rtc_handler()
Description: handles the interrupt by sending eoi and select register C to reset 
this also calls test_interrupts for testing 
input:none
output:none
return value:none
side effects: for testing purposes, it writes to video memory everytime an interrupt happens. Sends eoi and selects register C for reset purposes  
*/
void rtc_handler()
{
	cli();	//disable interrupts  
	send_eoi(RTC_IRQ_NUMB);

	read_flag = 1; //tell rtc_read that we're done handling the interrupt
	outb(SELECT_C, CMOS_OUT); //if register C is not read after IRQ8 it 
	inb(CMOS_IN);		  //will not receive more interrupts so we have to do this to get more interrupts 

	sti();	//enable interrupts 
}

/*
uint32_t rtc_open(const uint8_t* filename)
Description: opens rtc so that it can start taking interrupts
input: filename
output: none 
return value: 0 always
side effects: enables rtc interrupt 
*/
int32_t rtc_open(const uint8_t* filename) {
	init_rtc(); //initialize
	return 0;
}

/*
uint32_t rtc_close(uint32_t fd)
Description: close rtc so that it stops taking interrupts
input: fd
output: none 
return value: 0 always
side effects: disables rtc interrupt 
*/
int32_t rtc_close(int32_t fd) {
	disable_irq(RTC_IRQ_NUMB); //disable irq
	return 0;
}

/*
uint32_t rtc_read(uint32_t fd, void* buf, uint32_t nbytes)
Description: returns 0 when the interrupt flag has been cleared by the interrupt handler
input: fd, buf, nbytes
output: none
return value: 0 always
side effects: none 
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
	while(!read_flag);
	read_flag = 0;  //reset flag
	return 0;
}
 /*
int PowerTwo(int n)
Description: Function to check if x is power of 2
input: n - number to check
output: none
return value: 0 or 1 depending on if it's a power of 2 or not
side effects: none 
*/
int PowerTwo(int n)
{
  if (n == 0)
    return 0;
  while (n != 1)
  {
      if (n % 2 != 0)
         return 0; //return 0 if not power of 2
      n = n / 2;
  }
  return 1; //return 1 if power of 2
}

/*
uint32_t rtc_write(uint32_t fd, const void* buf, uint32_t nbytes) 
Description: writes desired frequency into rtc. Frequency must be power of 2, less than 1024, greater than 
input: fd, buf, nbytes
output: none
return value: -1 if there is an error, nbytes otherwise
side effects: none 
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
	if((nbytes != 4) || buf == NULL) //if number of bytes is not 4 or pointer is NULL, error
		return -1;
	
	uint32_t freq;
	freq = *((uint32_t*)buf);
	if(PowerTwo(freq) == 0 || freq > MAX_FREQ || freq < MIN_FREQ)  //check if power of 2, above 1024, below 2
		return -1;

	set_rate(freq); //set the rate
	return nbytes;
}


