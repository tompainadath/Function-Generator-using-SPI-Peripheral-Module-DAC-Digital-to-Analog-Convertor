#include "xparameters.h"
#include "xil_io.h"
#include "xil_printf.h"
#include <xgpio.h>
#include <math.h>

//DAC2Pmod from Address Editor in Vivado
#define DA2acq 0x43C00000 //DA2 acquisition - output
#define DA2dav 0x43C00004 //DA2 data available - input
#define DA2dat1 0x43C00008 //DA2 channel 1 data - output
#define DA2dat2 0x43C0000C //DA2 channel 2 data - output

#define BTN_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define SW_ID XPAR_AXI_GPIO_0_DEVICE_ID

#define BTN_CHANNEL 1
#define SW_CHANNEL 2

XGpio BTNInst, SWInst;

float amplitude = 2000;
double FREQ = 5;
#define FS 330000
#define offset 2048


int main(void)
{
	int status;
	status = XGpio_Initialize(&BTNInst, BTN_ID);
	if( status != XST_SUCCESS) return XST_FAILURE;

	status = XGpio_Initialize(&SWInst, SW_ID);
	if( status != XST_SUCCESS) return XST_FAILURE;

	XGpio_SetDataDirection(&BTNInst,1, 0xFF);
	XGpio_SetDataDirection(&SWInst,2, 0xFF);

	int dacdata1=0; //DAC ramp data
	int dacdata2=0; //DAC ramp data
	int dacdav = 0; //DAC data available
	//int dacacq=0; //DAC acquire
	xil_printf("\n\rStarting DA2 Pmod demo test...\n\r");
	Xil_Out32(DA2acq,0); //stop DAC acquire

	Xil_Out32(DA2acq,0);
	dacdav = Xil_In32(DA2dav);

	while(dacdav == 1)
		dacdav = Xil_In32(DA2dav);

	int outSamp = 0;
	int indexer = 0;

	int priority_btn = 0;

	while(1)
	{


		int btn_read = XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL);

		int sw_read = XGpio_DiscreteRead(&SWInst, SW_CHANNEL);
		amplitude = (sw_read * 136.466);  // Amplitude


		if ((btn_read == 0)){
			priority_btn = 0;
			FREQ = 10;
		}
		else if((btn_read == 1)){
			priority_btn = 1;
			btn1:
			FREQ = 0.5;
			for (int Delay = 0; Delay < 250; Delay++);
		}
		else if((btn_read == 2)){
			priority_btn = 2;
			btn2:
			FREQ = 1;
		}

		else if((btn_read == 4)){
			priority_btn = 3;
			btn3:
			FREQ = 2;
		}

		else if((btn_read == 8)){
			priority_btn = 4;
			btn4:
			FREQ = 5;
		}
		else{
			if (priority_btn == 1){
				goto btn1;
			}
			else if (priority_btn == 2){
				goto btn2;
			}
			else if (priority_btn == 3){
				goto btn3;
			}
			else if (priority_btn == 4){
				goto btn4;
			}
		}




		outSamp = floor(amplitude*sin((2*M_PI*FREQ*indexer)/FS))+offset; // Sine function
		indexer =(indexer+1)%(FS);  // Time

		dacdata1 = outSamp;
		dacdata2 = outSamp;

		Xil_Out32(DA2dat1, dacdata1); //output DAC data
		Xil_Out32(DA2dat2, dacdata2); //output DAC data
		Xil_Out32(DA2acq,1); //DAC acquire

		while (dacdav==0) //DAC data available
			dacdav = Xil_In32(DA2dav); //stop DAC acquire
		Xil_Out32(DA2acq, 0);
		while(dacdav==1)
			dacdav = Xil_In32(DA2dav);
	}
}
