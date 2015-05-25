/*******************************************************************************
 * Copyright (c) 2014-2015 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *******************************************************************************/
#include "board.h"
#include "radio.h"
#include "lmic.h"
#include "debug.h"
#include <string.h>

#define MAX_COUNTER  50 /// max value for counter of edges

extern void initsensor(osjobcb_t callback);
extern u2_t readsensor(void);


u1_t on=1;
u1_t counter=0;
u1_t joined=0;
//u1_t const MAX_COUNTER = 50;
// sensor functions

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////


// application router ID (LSBF)
static  u1_t APPEUI[8]  = { 0x3F, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xDE, 0xDE };

// unique device ID (LSBF)
static  u1_t DEVEUI[8]  = { 0x02, 0x00, 0x00, 0x3F, 0x00, 0x00, 0xD1, 0xC3 };

// device-specific AES key (derived from device EUI)
static  u1_t DEVKEY[16] = {	0xA8, 0xAA, 0xAA, 0x95, 0xAA, 0xAA, 0x7B, 0x69, 
														0x57, 0x55, 0x55, 0x6A, 0x55, 0x55, 0x84, 0x96};


//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}


//////////////////////////////////////////////////
// SENSOR CALLBACK (run by sensor IRQ job)
//////////////////////////////////////////////////
   u1_t ledstatus=0;
// report sensor value when change was detected
static void sensorfunc (osjob_t* j) {

		// read sensor
    u2_t val = readsensor();
    debug_val( "val = " , val);
	
		if (joined==1){
				counter++;
				if (counter == MAX_COUNTER){
					counter=0;							
					//ledstatus=!ledstatus;
					set_led(0,1); // red led on
					set_led(1,3);// turn on green led (3)
					LMIC.frame[0] = LMIC.snr;
					LMIC_setTxData2(1, LMIC.frame, 1, 0);
					//LMIC_setTxData2(1, LMIC.frame, 2, 0); // (port 1, 2 bytes, unconfirmed)
				}
				else{
					set_led(1,1); // red led on
					set_led(0,3);// turn off green led (3)
				}
			
		}
    // prepare and schedule data for transmission
    //LMIC.frame[0] = val << 8;
    //LMIC.frame[1] = val;
    
}


//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// initial job
static void initfunc (osjob_t* j) {
    // intialize sensor hardware
    initsensor(sensorfunc);
    // reset MAC state
    LMIC_reset();
	
	  // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}


// application entry point
int main () {
    osjob_t initjob;

    // initialize runtime env
    os_init();
    // initialize debug library
    debug_init();
			
    // setup initial job
    os_setCallback(&initjob, initfunc);
		//LMIC_setTxData2(1, LMIC.frame, 1, 0);
		//GpioWrite( &Led2, 1 );	
		
    // execute scheduled jobs and events
    os_runloop();
    // (not reached)
    return 0;
}



//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////

static osjob_t blinkjob;
static u1_t ledstate = 0;

static void blinkfunc (osjob_t* j) {
    // toggle LED
    ledstate = !ledstate;
    debug_led(ledstate);
		
    // reschedule blink job
    os_setTimedCallback(j, os_getTime()+ms2osticks(100), blinkfunc);
}



//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {

			  // starting to join network
      case EV_JOINING:
          // start blinking
          blinkfunc(&blinkjob);// orange led flashing
          break;
          
      // network joined, session established
      case EV_JOINED:
          // cancel blink job
          os_clearCallback(&blinkjob);
          // switch on orange LED 
          debug_led(1); 
					joined=1;
					//LMIC.frame[0] = LMIC.snr;
					
          // (don't schedule any new actions)
          break;
			
			case EV_TXCOMPLETE:
          
			
					if(TXRX_ACK) {
						
						set_led(1,2); //yellow led
						
					}
					if(LMIC.dataLen) { // data received in rx slot after tx
							//set_led(1,3); //green led
							// cancel blink job
							os_clearCallback(&blinkjob);
              debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
							//debug_led(1);
						}
					/*LMIC.frame[0] = LMIC.snr;
					LMIC.frame[1] = LMIC.rssi;
					LMIC_setTxData2(1, LMIC.frame, 2, 0);
					//set_led(0,2);
				  debug_val ("Frame: ", LMIC.frame[0]);
						*/
			case EV_RXCOMPLETE:
					if(TXRX_ACK) {
						set_led(0,2); //yellow led
						//blinkfunc(&blinkjob,3); //green led flashing
					}
						
						
			case EV_LINK_DEAD:
					debug_str ("Link Dead");
					//blinkfunc(&blinkjob,1);// red led flashing
       
    }
}
