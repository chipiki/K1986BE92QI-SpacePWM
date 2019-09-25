/*
 * main.c
 *
 *  Created on: 17 џэт. 2019 у.
 *      Author: DA.Tsekh
 */


#include <stdint.h>
#include <string.h>

#include "config.h"
#include "MDR32Fx.h"
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_port.h"

#include "SpacePWM.h"


int main (void){

	RST_CLK_FreqTypeDef RST_CLK_Clocks;

	// Configure MCU clocks
	clock_configure();

	// Enable used ports
	init_all_ports();

	// for rotating BLDC motor.
	SpacePWM_Timer3Config();

	// Just for checking MCU clocks in "RST_CLK_Clocks"
	RST_CLK_GetClocksFreq( &RST_CLK_Clocks );

	while(1){
		for (int i = 0; i < 65535; i++){
			// Use SpacePWM
			SpacePWM( 25, i++ );
			// just for delay
			for(int j=0; j<10; j++);
		}

	}

	return 0;
}

