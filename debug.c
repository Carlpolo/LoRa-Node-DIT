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

#include "hw.h"
#include "debug.h"
#include "lmic.h"

#define LED_PORT        GPIOA // use GPIO PA 
#define LED_PIN1        3 //use GPIO PA3 (LED1 on IMST)
#define LED_PIN2       	0 //use GPIO PA0 (LED2 on IMST)
#define LED_PIN3       	1 //use GPIO PA1 (LED0 on IMST)
#define LED_PIN4        8 //use GPIO PA8 (LED4 on IMST)

/*#define INPUT_PORT			GPIOA
#define INPUT_PIN				12 //button 1
*/
#define INPUT_PORT			GPIOB
#define INPUT_PIN				14 //button 2

#define OUTPUT_PORT			GPIOB
#define OUTPUT_PIN			15

#define USART_TX_PORT   GPIOA
#define USART_TX_PIN    9
/*///////////////////////////////////////////////////////////////
Added by Carlos 2015-06-08 for the configuration of USART_RX */
#define USART_RX_PORT   GPIOA
#define USART_RX_PIN    10 
////////////////////////////////////////////////////////////////
#define GPIO_AF_USART1  0x07







void debug_init () {
    // configure LED pin as output
		hw_cfg_pin(LED_PORT, LED_PIN1, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		hw_cfg_pin(LED_PORT, LED_PIN2, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		hw_cfg_pin(LED_PORT, LED_PIN3, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
    hw_cfg_pin(LED_PORT, LED_PIN4, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		
		//configure pin to use as Output -- Modified by Carlos 2015-06-11
		hw_cfg_pin(OUTPUT_PORT, OUTPUT_PIN, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);// pin for buzzer configured for output
		////////////////////////////////////////////////////////////
		//configure pin to use as Input -- Modified by Carlos 2015-06-15
		hw_cfg_pin(INPUT_PORT, INPUT_PIN, GPIOCFG_MODE_INP | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_OPEN);// pin for button2 configured for input
		//hw_cfg_pin(INPUT_PORT, INPUT_PIN, GPIOCFG_MODE_INP | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);// pin for button1 configured for imput
		///////////////////////////////////////////////////////////////////////////
		set_led(1,1); //to get the output for the relay 2015-06-15 Carlos
    debug_led(0);

    // configure USART1 (115200/8N1, tx-only)
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    hw_cfg_pin(USART_TX_PORT, USART_TX_PIN, GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIOCFG_OTYPE_PUPD|GPIOCFG_PUPD_PUP|GPIO_AF_USART1);
		//hw_cfg_pin(USART_RX_PORT, USART_RX_PIN, GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIOCFG_OTYPE_PUPD|GPIOCFG_PUPD_PUP|GPIO_AF_USART1);//added by Carlos 2015-06-08, configures the USART_RX
    USART1->BRR = 277; // 115200
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE; // usart+transmitter enable
		//USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;//usart+transmitter+receiver enable -- Modified by Carlos 2015-06-08
    // print banner
    /*
		Commented out by Carlos 2015-06-11
		debug_str("\r\n============== DEBUG STARTED ==============\r\n");
		*/
}
////////////modified by Carlos 2015-06-11
void debug_output_HIGH_LEVEL(){
	//hw_set_pin(OUTPUT_PORT, OUTPUT_PIN, 1);
	hw_set_pin(LED_PORT, LED_PIN2, 1);
}
void debug_output_LOW_LEVEL(){
	//hw_set_pin(OUTPUT_PORT, OUTPUT_PIN, 0);
	hw_set_pin(LED_PORT, LED_PIN2, 0);
}
/////////////////////////////////////////
/////////////////////////////////////////
/*void led(u1_t val) 
added by Carlos 2015-05-21 
to work with other LED's because void debug_led (u1_t val) only works with LED 4
*/
void set_led(u1_t val,u1_t n_led) {
	int LED_PIN;
		if (n_led == 1) LED_PIN=LED_PIN1;
		else if (n_led == 2) LED_PIN=LED_PIN2;
		else if (n_led == 3) LED_PIN=LED_PIN3;
		else LED_PIN=LED_PIN4;
    hw_set_pin(LED_PORT, LED_PIN, val);
}

////////////////////////////////////////////////////////////////
/*Modified by Carlos 2015-06-15--Reads the INPUT GPIO*/

u2_t read_STATUS (){
	 /*if ((GPIOB->IDR & (1 << INPUT_PIN)) != 0)
	//if ((GPIOB->IDR & INPUT_PIN) != 0)
		 return 1;
	 else
		 return 0;
	 */
	 return ((GPIOB->IDR & (1 << INPUT_PIN)) != 0);
	
}

////////////////////////////////////////////////////////////////
void debug_led (u1_t val) {
    hw_set_pin(LED_PORT, LED_PIN4, val);
}

void debug_char (u1_t c) {
    while( !(USART1->SR & USART_SR_TXE) );  
		//USART1->DR = c;
    USART1->DR = c&0xff; //modified by Carlos 2015-06-11
		
		
}


void debug_hex (u1_t b) {
    debug_char("0123456789ABCDEF"[b>>4]);
    debug_char("0123456789ABCDEF"[b&0xF]);
}

void debug_buf (const u1_t* buf, u2_t len) {
    while(len--) {
        debug_hex(*buf++);
        debug_char(' ');
    }
    debug_char('\r');
    debug_char('\n');
}

void debug_uint (u4_t v) {
    for(s1_t n=24; n>=0; n-=8) {
        debug_hex(v>>n);
    }
}

void debug_str (const u1_t* str) {
    while(*str) {
        debug_char(*str++);
    }
}

void debug_val (const u1_t* label, u4_t val) {
    debug_str(label);
    debug_uint(val);
    debug_char('\r');
    debug_char('\n');
}

void debug_event (int ev) {
    static const u1_t* evnames[] = {
        [EV_SCAN_TIMEOUT]   = "SCAN_TIMEOUT",
        [EV_BEACON_FOUND]   = "BEACON_FOUND",
        [EV_BEACON_MISSED]  = "BEACON_MISSED",
        [EV_BEACON_TRACKED] = "BEACON_TRACKED",
        [EV_JOINING]        = "JOINING",
        [EV_JOINED]         = "JOINED",
        [EV_RFU1]           = "RFU1",
        [EV_JOIN_FAILED]    = "JOIN_FAILED",
        [EV_REJOIN_FAILED]  = "REJOIN_FAILED",
        [EV_TXCOMPLETE]     = "TXCOMPLETE",
        [EV_LOST_TSYNC]     = "LOST_TSYNC",
        [EV_RESET]          = "RESET",
        [EV_RXCOMPLETE]     = "RXCOMPLETE",
        [EV_LINK_DEAD]      = "LINK_DEAD",
        [EV_LINK_ALIVE]     = "LINK_ALIVE",
    };
    debug_str(evnames[ev]);
    debug_char('\r');
    debug_char('\n');
}
