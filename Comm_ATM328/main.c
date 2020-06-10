/*
 * Comm_ATM328.c
 * Created: 29.11.2019 
 * Author : Rusaleev MB
 */ 

/******************************************** Library ***********************************************/
#include "main.h"
/*FUSE bit Boorst ON, boot 2048*/
// http://arduino.ru/forum/apparatnye-voprosy/ne-mogu-vylechit-vatchdog-na-goloi-atmege328r-s-vnutr-taktirovaniem-8-mgts
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) \
__attribute__((naked)) \
__attribute__((section(".init3")));
void get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

uint8_t in_crc_err=5;
extern uint8_t status;
uint8_t fn=8;
//uint32_t amout_mb=505;
//uint32_t  amout_mb_error=123;
/******************************************** Main cycle *****************************************/
int main(void)
{
	wdt_disable();
	
	//char str_temp[80]={'\0'};
	//init_uart_mb();
	
	gpio_init();
	LED_RG_ON
	unload_eeprom_data();
	
	_delay_ms(1000);
	modbusInit();
	SPI_Init_Slave();
	timer_counter_init();
		
	//status=18;
	
	create_new_messages(&in_crc_err,&status,&fn,&eeprom_num_req,&eeprom_num_err);
	sei();

	LED_OFF
	LED_G_ON;
	wdt_enable(WDTO_2S);  //Включение WDT
	/*sprintf(str_temp,"MB-%d\r\nNum_boot-%d\r\nreq-%lu\r\nerr-%lu\r\n%s\r\n",eeprom_modbus_adress,num_boot_Sys,eeprom_num_req,eeprom_num_err,eeprom_soft_version);
	printStr_mb(str_temp);*/

	//printStr_mb("READY\r\n");
    while (1) 
    {
		wdt_reset(); //сброс флага WDT
		
		modbusGet(); ///Проверка входящих сообщений
		wdt_reset(); //сброс флага WDT
		/*if (SPI_STAT)
		{
			printStr_mb("Ok\r\n");
			SPI_STAT=0;
		}
		else if(SPI_STAT==99)
		{
			printStr_mb("Er\r\n");
			SPI_STAT=0;
		}*/

		if ((rx_completed_spi==1)&&(!(modbusGetBusState() & (1<<2))&&(!(modbusGetBusState() & (1<<1)))&&(!(modbusGetBusState() & (1<<7)))))
		{
			if (check_in_crc16(incoming_message,&in_crc_err))
			{
				convert_incoming_message(incoming_message,ai_float, &in_modbus_adress,&DO_status, &in_status);
			}
	
			rx_completed_spi=0;
		}
		//_delay_ms(100);
		if (ready_for_processing)
		{	
				
			wdt_reset(); //сброс флага WDT
			create_new_messages(&in_crc_err,&status,&fn,&eeprom_num_req,&eeprom_num_err);
			ready_for_processing=0;
				
		}
		if (ready_for_update)
		{
			update_eeprom_counters(&eeprom_num_req,&eeprom_num_err);
			ready_for_update=0;
		}

		_delay_us(100);
	}
}

