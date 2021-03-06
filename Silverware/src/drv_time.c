//
#include "project.h"
#include "drv_time.h"
#include "config.h"

void failloop( int val);

unsigned long lastticks;
unsigned long globalticks;
volatile unsigned long systickcount = 0;


#ifndef SYS_CLOCK_FREQ_HZ
#define SYS_CLOCK_FREQ_HZ 48000000
#warning SYS_CLOCK_FREQ_HZ not present
#endif


 // divider by 8 is enabled in this systick config
static __INLINE uint32_t SysTick_Config2(uint32_t ticks)
{
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1);            /* Reload value impossible */

  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Cortex-M0 System Interrupts */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = //SysTick_CTRL_CLKSOURCE_Msk |   // divide by 8
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}


void time_init()
{
    if (SysTick_Config2( SYS_CLOCK_FREQ_HZ /8 ))
    {// not able to set divider
        failloop(5);
    }
}

// return time in uS from start ( micros())
// called at least once per second or time will overflow
unsigned long gettime(void)
{

    unsigned long maxticks = SysTick->LOAD;
    unsigned long ticks = SysTick->VAL;
    unsigned long elapsedticks;

    static unsigned long remainder = 0; // carry forward the remainder ticks;

    if (ticks < lastticks) elapsedticks = lastticks - ticks;
    else
    {// overflow ( underflow really)
        elapsedticks = lastticks + ( maxticks - ticks);
    }

    lastticks = ticks;
    elapsedticks += remainder;

    #ifdef ENABLE_OVERCLOCK
        const unsigned long quotient = elapsedticks / 8;
        remainder = elapsedticks - quotient * 8;
    #else
        unsigned long quotient = elapsedticks / 6;
        remainder = elapsedticks - quotient * 6;
    #endif
    globalticks += quotient;
    return globalticks;
}



#ifdef ENABLE_OVERCLOCK
// delay in uS
void delay(uint32_t data)
{
	volatile uint32_t count;
	count = data * 7;
	while (count--);
}
#else
// delay in uS
void delay(uint32_t data)
{
	volatile uint32_t count;
	count = data * 5;
	while (count--);
}
#endif

void SysTick_Handler(void)
{

}
