/*
 * SpacePWM.c
 *
 *  Created on: 21 июн. 2018 г.
 *      Author: DA.Tsekh
 */


#include "MDR32Fx.h"
#include "MDR32F9Qx_timer.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"

#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_ssp.h"

#include "config.h"

#ifdef USE_SVPWM_BLDC

#include "SpacePWM.h"

#define T_05_PWM 249   //1/2 периода ШИМ

void SpacePWM_Timer3Config( void );

const unsigned short tab_Sin_SPwm[257] = {
0,268,536,804,1072,1340,1608,1876,2144,2412,2680,2947,3215,3483,3751,4018,
4286,4553,4821,5088,5355,5622,5889,6156,6423,6690,6957,7223,7489,7756,8022,8288,
8554,8819,9085,9350,9616,9881,10146,10410,10675,10939,11204,11468,11732,11995,12259,12522,
12785,13048,13310,13573,13835,14097,14359,14620,14881,15142,15403,15663,15923,16183,16443,16702,
16961,17220,17479,17737,17995,18253,18510,18767,19024,19280,19536,19792,20047,20302,20557,20811,
21065,21319,21572,21825,22078,22330,22582,22833,23085,23335,23586,23836,24085,24334,24583,24831,
25079,25327,25574,25820,26066,26312,26557,26802,27047,27291,27534,27777,28020,28262,28503,28745,
28985,29226,29465,29704,29943,30181,30419,30656,30893,31129,31365,31600,31834,32069,32302,32535,
32767,32999,33231,33462,33692,33921,34150,34379,34607,34834,35061,35287,35513,35738,35962,36186,
36409,36632,36854,37075,37296,37516,37736,37955,38173,38390,38607,38824,39039,39254,39469,39682,
39895,40108,40319,40530,40741,40950,41159,41368,41575,41782,41988,42194,42398,42602,42806,43008,
43210,43412,43612,43812,44011,44209,44407,44603,44799,44995,45189,45383,45576,45768,45960,46150,
46340,46530,46718,46906,47092,47279,47464,47648,47832,48015,48197,48378,48558,48738,48917,49095,
49272,49448,49624,49799,49972,50146,50318,50489,50660,50829,50998,51166,51333,51499,51665,51829,
51993,52155,52317,52478,52639,52798,52956,53114,53270,53426,53581,53735,53888,54040,54191,54341,
54491,54639,54787,54933,55079,55224,55368,55511,55653,55794,55934,56073,56212,56349,56485,56621,
56755};


void SpacePWM(int Upwm, int Ndv)
{

	//Upwm - величина вектора (д.б. от -T_05_PWM до +T_05_PWM)
	//Ndv - 16 -ти разрядный код угла (д. б. от 0 до 2*pi (0-65535))

	int i1, i2;
	MDR_TIMER_TypeDef* timer;

	i1 = 6;
	i1 = Ndv * i1;
	Ndv = i1 >> 16;   //Ndv = 0-5
	Ndv -= 3;
	if (Ndv < 0)         //сектор 0,1,2
		Ndv += 3;         //Ndv = 0-2
	else
		//сектор 3,4,5; Ndv = 0-2
		Upwm = -Upwm;
	i1 &= 0xffff;
	i1 >>= 8;
	i2 = 0x100 - i1;
	i1 = tab_Sin_SPwm[i1];   //i1 = dy
	i2 = tab_Sin_SPwm[i2];   //i2 = dx
	i1 = Upwm * i1;
	i2 = Upwm * i2;
	i1 >>= 16;
	i2 >>= 16;
	Upwm = i1;
	i1 = i2 + i1;               //i1 = Upwm*(dx+dy)
	//i2 = Upwm - i2;               //i2 = Upwm*(dx-dy)
	i2 = i2 - Upwm;               //i2 = Upwm*(dx-dy)
	timer = MDR_TIMER3;
	Upwm = T_05_PWM;
	if (Ndv)               //Ndv = 0,1,2
	{
		if (Ndv == 1)   //сектор 1 или 4
		{
			timer->CCR1 = Upwm - i2;      //TA
			timer->CCR2 = Upwm - i1;      //TB
			timer->CCR3 = Upwm + i1;      //TC
		} else               //сектор 2 или 5
		{
			timer->CCR1 = Upwm + i1;      //TA
			timer->CCR2 = Upwm - i1;      //TB
			timer->CCR3 = Upwm + i2;      //TC
		}
	} else                  //сектор 0 или 3
	{
		timer->CCR1 = Upwm - i1;      //TA
		timer->CCR2 = Upwm + i2;      //TB
		timer->CCR3 = Upwm + i1;      //TC
	}


}


void SpacePWM_Timer3Config( void )
{
	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER3, DISABLE );
	TIMER_DeInit( MDR_TIMER3 );

	/*
	 * Настройка GPIO
	 */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE );
	static PORT_InitTypeDef PORT_InitStructure;
	/* Configure PORTA pin 9 for output PWM */
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_Pin = ( PORT_Pin_1 | PORT_Pin_2 | PORT_Pin_3 );
	PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
	PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTB, &PORT_InitStructure);

	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_Pin = ( PORT_Pin_5 | PORT_Pin_6 );
	PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
	PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTB, &PORT_InitStructure);

	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTC, ENABLE );
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_Pin = ( PORT_Pin_2 );
	PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
	PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTC, &PORT_InitStructure);


	MDR_RST_CLK_TypeDef* rst_clk = MDR_RST_CLK;
	MDR_TIMER_TypeDef* timer = MDR_TIMER3;

	rst_clk->PER_CLOCK |= (1 << 19); //TIMER 3
	rst_clk->TIM_CLOCK = (1 << 26); // TIMER3 (HCLK)

	timer->PSG = 1;
	timer->ARR = 1000; // PWM: (80MHz / (PSG+1)) / 1000 / 2 = 20 KHz
	timer->CCR1 = 500 + 499;
	timer->CCR2 = 500;
	timer->CCR3 = 500 - 499;

	timer->CH1_CNTRL = 7 << 9; //ref = 1 cnt < ccr
	timer->CH2_CNTRL = 7 << 9; //ref = 1 cnt < ccr
	timer->CH3_CNTRL = 7 << 9; //ref = 1 cnt < ccr

	timer->CH1_DTG = 0x02 << 0; //dtg = 10
	timer->CH2_DTG = 0x02 << 0; //dtg = 10
	timer->CH3_DTG = 0x02 << 0; //dtg = 10

	timer->CH1_DTG = 0xFE << 8; //dtg = 10
	timer->CH2_DTG = 0xFE << 8; //dtg = 10
	timer->CH3_DTG = 0xFE << 8; //dtg = 10

	timer->CH1_CNTRL1 = (1 << 0) | (3 << 2) | (1 << 8) | (3 << 10); //ref out dtg
	timer->CH2_CNTRL1 = (1 << 0) | (3 << 2) | (1 << 8) | (3 << 10); //ref out dtg
	timer->CH3_CNTRL1 = (1 << 0) | (3 << 2) | (1 << 8) | (3 << 10); //ref out dtg

	timer->CNTRL = (1 << 0) | (1 << 6); //timer3 0n Up/Down

}

#endif
