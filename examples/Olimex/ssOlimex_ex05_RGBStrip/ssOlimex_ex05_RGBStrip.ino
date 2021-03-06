/**************************************************************************
	Souliss - Control RGB LED Strip
	
	It control via Android an RGB LED or Strip using the MOD-RGB extension
	module, the communication goes over Ethernet using MOD-ENC28J60 or 
	MOD-WIFI.
	
	This code allow the manual control of the LED using a color picker, or
	drive the LED color using the music played by your smartphone.

	To use MOD-RGB and a communication board (MOD-ENC28J60 or MOD-WIFI)
	is required a ribbon cable that extend the UEXT bus to both the boards,
	the signal dedicated for I2C shall not be extended to the communication
	modules.
	
	CONFIGURATION IS MANDATORY BEFORE COMPILING
	Before compiling this code, is mandatory the configuration of the framework
	this ensure the use of proper drivers based functionalities and requested
	communication interface.	
	
	Configuration files are located on /conf folder, is suggested to use this 
	code on one of the boards listed below.	
	
	Run this code on one of the following boards:
	
		Board Conf Code			Board Model
		0x19		Olimex AVR-T32U4 		with MOD-ENC28J60 and MOD-RGB(UEXT)
		0x1A		Olimex OLIMEXINO-32U4 	with MOD-ENC28J60 and MOD-RGB(UEXT)
		0x1B		Olimex OLIMEXINO-328 	with MOD-ENC28J60 and MOD-RGB(UEXT)
		0x1C		Olimex AVR-T32U4 		with MOD-WIFI	  and MOD-RGB(UEXT)
		0x1D		Olimex OLIMEXINO-32U4 	with MOD-WIFI	  and MOD-RGB(UEXT)
		0x1E		Olimex OLIMEXINO-328 	with MOD-WIFI	  and MOD-RGB(UEXT)		
	
	******************** Configuration Parameters *********************
	
		Configuration file		Parameter
		QuickCfg.h				#define	QC_ENABLE			0x01
		QuickCfg.h				#define	QC_BOARDTYPE		0x19, 0x01A, 0x1B,
															0x1C, 0x1D, 0x1E

	Is required an additional IP configuration using the following parameters
		QuickCfg.h				const uint8_t DEFAULT_BASEIPADDRESS[] = {...}
		QuickCfg.h				const uint8_t DEFAULT_SUBMASK[]       = {...}
		QuickCfg.h				const uint8_t DEFAULT_GATEWAY[]       = {...}
	
***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"

// network addresses
#define network_address_1	0x0011 			// 0x0011 is equal to 17 in decimal
#define network_my_subnet	0xFF00
#define network_my_supern	0x0000

#define SLOT_RGB_STATE		0					// This is the memory slot used for the execution of the logic
#define	SLOT_RGB_COLOR		SLOT_RGB_STATE+1	// This is the memory slot used for the execution of the logic

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  

void setup()
{	
	// Setup the network configuration
	//
	//	The vNet address is 11(hex) that is 17(dec), so the IP address is
	//	the DEFAULT_BASEIPADDRESS[] defined in ethUsrCfg.h plus 17 on last 
	//  octect. If DEFAULT_BASEIPADDRESS[] = {192, 168, 1, 0} the IP address
	//  for the board will be 192.168.1.17
	Souliss_SetAddress(network_address_1, network_my_subnet, network_my_supern);		

	// Load the address also in the memory_map
	Souliss_SetLocalAddress(memory_map, network_address_1);	
	
	// Set the typical logic to use, T16 is a color picker and music analyzer
	Souliss_SetT16(memory_map, SLOT_RGB_STATE);
	
	// Init the MOD-RGB module
	Souliss_MODRGB_Init();
	
}

void loop()
{ 
	// The Souliss methods are scheduled in phases, this allow load
	// balance and proper timing.

	if(abs(millis()-tmr_fast) > time_base_fast)
	{	
		tmr_fast = millis();
		phase_fast = (phase_fast + 1) % num_phases;
		
		// Execute the code every 1 time_base_fast		
		if (!(phase_fast % 1))
		{		
			// Retreive data from the communication channel
			Souliss_CommunicationData(memory_map, &data_changed);
			
		} 		

		// Execute the code every 5 time_base_fast		
		if (!(phase_fast % 5))
		{
			// Execute the logic
			Souliss_Logic_T16(memory_map, SLOT_RGB_STATE, &data_changed);
									
			// Set outputs
			Souliss_MODRGB_SetColor(&Souliss_Output(memory_map, SLOT_RGB_COLOR));
			
		} 
		
	}
	else if(abs(millis()-tmr_slow) > time_base_slow)
	{	
		tmr_slow = millis();
		phase_slow = (phase_slow + 1) % num_phases;

		// Execute the code every time_base_slow
		if (!(phase_slow % 1))
		{                 
			// The timer handle timed-on states
			Souliss_T16_Timer(memory_map, SLOT_RGB_STATE);						
		} 	  
	}	
} 