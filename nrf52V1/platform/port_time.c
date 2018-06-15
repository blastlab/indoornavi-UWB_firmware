/*
 * port_time.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"

//#define LPTIM_SLEEP LPTIM1 TODO: implement timers handlers
//#define LPTIM_SLOT LPTIM2

void PORT_TimeInit() {

}

void PORT_TimeStartTimers() {

}

void PORT_SleepMs(unsigned int time_ms) {

}

unsigned int PORT_TickMs() { return 0; }	// TODO: implement tickMs

// get high resosolution clock tick

unsigned int PORT_TickHr() { return 0; }	// TODO: implement tickHr

unsigned int PORT_TickHrToUs(unsigned int delta) {
	return 0;
 }

// update slot timer for one iteration, @us is us to the next IT
void PORT_SlotTimerSetUsLeft(uint32 us) {

}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {

}
