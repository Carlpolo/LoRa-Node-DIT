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
#include "stdio.h"
#include "stdlib.h"
#include "board.h"
#include "radio.h"
#include "lmic.h"
#include "debug.h"
#include <string.h>
#include <limits.h>
#include "delay.h"

#define MAX_COUNTER  20 /// max value for counter of edges

extern void initsensor(osjobcb_t callback);
extern u2_t readsensor(void);


u1_t *received_data;
u1_t on=1;
u1_t counter=0;
u1_t joined=0;
u2_t rx=0;

///////////////////////////////////////////////////////
//List of commands

char *cmd_turn_on_led1 = "Led1";
char *cmd_turn_on_led2 = "Led2";
char *cmd_Read_UART = "UART_ReadStatus";
char *cmd_Write_UART = "UART_Write";

/////////////////////////////////////////////////////
const int max_int = INT_MAX;
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
static  u1_t DEVKEY[16] = {0xA8, 0xAA, 0xAA, 0x95, 0xAA, 0xAA, 0x7B, 0x69, 
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
    //debug_val( "val = " , val);
	
		if (joined == 1){
				counter++;
				if (counter == MAX_COUNTER){
					counter=0;							
					
					LMIC.frame[0] = LMIC.rssi;
					LMIC.frame[1] = LMIC.snr;
					
					//LMIC_setTxData2(1, received_data, sizeof(rx), 1); //send the rx data 
					LMIC_setTxData2(1, LMIC.frame, 2, 0); // (port 1, 2 bytes, unconfirmed)
				}
				else{
					//set_led(1,1); // red led on
					set_led(0,3);// turn off green led (3)
				}
			
		}
    
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
		if (joined == 0)
			{
				
				
				
				debug_led(ledstate);
			}
		else 
			{
				set_led(ledstate,2);
			}
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
					
					
          // (don't schedule any new actions)
          break;
			
			case EV_TXCOMPLETE:
          
			
					if(TXRX_ACK) {
						// it goes here! -- Carlos 2015-06-02
						set_led(1,3);// turn on green led (3)
					
					}
					if(LMIC.dataLen) { // data received in rx slot after tx
						
						received_data = LMIC.frame+LMIC.dataBeg;
						////////////////////////////////////////////////////////
						/* Added by Carlos 2015-06-08 */
						
						if ( strstr(received_data, cmd_Write_UART) != NULL )
							{
								
								//set_led(1,1);//turn on led1
								//set_led(1,2);//turn on led2
								rx = read_STATUS();
								
								if (rx == 1){
									debug_output_LOW_LEVEL();
								}
								else{
									debug_output_HIGH_LEVEL();
								}
								//debug_char(0);
								hal_waitUntil(os_getTime()+ms2osticks(1000));
							
									
							}
						if ( strstr(received_data, cmd_Read_UART) != NULL )
							{
							
							//set_led(1,1);//turn on led1
							//set_led(1,2);//turn on led2
								
							rx = read_STATUS();	
							
							received_data = (u1_t *)&rx;
							
							LMIC_setTxData2(1, received_data, sizeof(rx), 1); //send the rx data 
							
						}
						////////////////////////////////////////////////////////
					
						if ( strstr(received_data, cmd_turn_on_led1) != NULL  ) // if command received is "Led1"
								{
									set_led(1,1);//turn on led1
									set_led(0,2);//turn off led2
									LMIC_setTxData2(1, received_data, LMIC.dataLen, 1); //send back the received data 
									
								}
						
							if ( strstr(received_data, cmd_turn_on_led2) != NULL  ) // if command received is "Led2"
								{
									set_led(1,2);//turn on led2
									set_led(0,1);//turn off led1
									LMIC_setTxData2(1, received_data, LMIC.dataLen, 1); //send back the received data 
									
								}
								
						}
					break;
	
			
						
			case EV_LINK_DEAD:
					debug_str ("Link Dead");
					//blinkfunc(&blinkjob,1);// red led flashing
					break;
    }
}