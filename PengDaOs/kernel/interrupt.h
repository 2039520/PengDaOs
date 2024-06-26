#ifndef _INTERRUPT_H_PD
#define _INTERRUPT_H_PD
#include "linkage.h"

void init_interrupt();

void do_IRQ(unsigned long regs,unsigned long nr);

#endif
