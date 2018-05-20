#define ASM 1
#include "x86_desc.h"

.globl	keyboard_wrapper, rtc_wrapper, syscall_wrapper


  # keyboard_wrapper()
  #	  DESCRIPTION: Assembly wrapper for keyboard irq handler. Necessary for IRET
  #	  INPUT: none
  #   OUTPUT: none
  #	  RETURN VALUE: none
  #	  SIDE EFFECTS: calls keyboard irq handler

keyboard_wrapper:

	pushal
	pushfl
	call	keyboard_handler
	popfl
	popal
	iret

  # rtc_wrapper()
  #	  DESCRIPTION: Assembly wrapper for rtc irq handler. Necessary for IRET
  #	  INPUT: none
  #   OUTPUT: none
  #	  RETURN VALUE: none
  #	  SIDE EFFECTS: calls rtc irq handler	
rtc_wrapper:
	pushal
	pushfl
	call	rtc_handler
	popfl
	popal
	iret

  # syscall_wrapper()
  #	  DESCRIPTION: 
  #	  INPUT: 
  #   OUTPUT: 
  #	  RETURN VALUE: 
  #	  SIDE EFFECTS: 	
syscall_wrapper:

	#pushal
	#check if invalid arg
	cmp 	$0, %eax
	jle 	error
	cmp 	$10, %eax
	jg 		error
	jmp 	*syscalls(,%eax,4)


  # *____* call()
  #	  DESCRIPTION: calls whatever function  
  #	  INPUT: %ebx, (sometimes %ecx %edx depending on number of arguments)
  #   OUTPUT: none
  #	  RETURN VALUE: %eax 
  #	  SIDE EFFECTS: calls the system call 

haltcall:
	pushl 	%ebx
	call 	halt
	popl 	%ebx
	jmp 	done
	
executecall:
	pushl 	%ebx
	call 	execute
	popl 	%ebx
	jmp 	done
	
readcall:
	pushl 	%edx
	pushl 	%ecx
	pushl 	%ebx
	call 	read
	popl 	%ebx
	popl 	%ecx
	popl 	%edx
	jmp 	done
	
writecall:
	pushl 	%edx
	pushl 	%ecx
	pushl 	%ebx
	call 	write
	popl 	%ebx
	popl 	%ecx
	popl 	%edx
	jmp 	done
	
opencall:
	pushl 	%ebx
	call 	open
	popl 	%ebx
	jmp 	done
	
closecall:
	pushl 	%ebx
	call 	close
	popl 	%ebx
	jmp 	done
	
getargscall:
	pushl 	%ecx
	pushl 	%ebx
	call 	getargs
	popl 	%ebx
	popl 	%ecx
	jmp 	done
	
vidmapcall:
	pushl 	%ebx
	call 	vidmap
	popl 	%ebx
	jmp 	done
	
set_handlercall:
	pushl 	%ecx
	pushl 	%ebx
	call 	set_handler
	popl 	%ebx
	popl 	%ecx
	jmp done
	
sigreturncall:
	call 	sigreturn
	jmp 	done
	
error:
	movl	$-1, %eax
done:
	iret

#system calls jump table
syscalls:
	.long 0, haltcall, executecall, readcall, writecall, opencall, closecall, getargscall, vidmapcall, set_handlercall, sigreturncall
