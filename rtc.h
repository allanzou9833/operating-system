#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "i8259.h"

#define BIT_6 0x40
#define MAX_FREQ 1024
#define MIN_FREQ 2
#define CMOS_OUT 0x70
#define CMOS_IN 0x71
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define SELECT_C 0x0C
#define RTC_IRQ_NUMB 8

void set_rate(uint32_t freq); //set rate 
void init_rtc(); //initialize rtc 
void rtc_handler(); //handles rtc 
int32_t rtc_open(const uint8_t* filename); //opens rtc file
int32_t rtc_close(int32_t fd); //closes
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes); //waits till interrupt has been handled 
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes); //writes desired frequency into rtc 
#endif

