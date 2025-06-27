#include "timer.h"
#include "../hal/cpu/lapic.h"

void timer_recalibrate()
{
    LAPIC_timer_init(0x20, 0xFFFFFFFF);
}

void timer_init()
{

}