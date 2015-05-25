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




#define USART_TX_PORT   GPIOA
#define USART_TX_PIN    9
#define GPIO_AF_USART1  0x07

void debug_init () {
    // configure LED pin as output
		hw_cfg_pin(LED_PORT, LED_PIN1, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		hw_cfg_pin(LED_PORT, LED_PIN2, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		hw_cfg_pin(LED_PORT, LED_PIN3, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
    hw_cfg_pin(LED_PORT, LED_PIN4, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
		
    debug_led(0);

    // configure USART1 (115200/8N1, tx-only)
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    hw_cfg_pin(USART_TX_PORT, USART_TX_PIN, GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIOCFG_OTYPE_PUPD|GPIOCFG_PUPD_PUP|GPIO_AF_USART1);
    USART1->BRR = 277; // 115200
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE; // usart+transmitter enable

    // print banner
    debug_str("\r\n============== DEBUG STARTED ==============\r\n");
}

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
////////////////////////////////////



void debug_led (u1_t val) {
    hw_set_pin(LED_PORT, LED_PIN4, val);
}

void debug_char (u1_t c) {
    while( !(USART1->SR & USART_SR_TXE) );    
    USART1->DR = c;
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
