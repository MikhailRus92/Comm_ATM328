/******************************************** Library ***********************************************/
#include "eeprom.h"
/******************************************** Variables ********************************************/
char eeprom_test[32] =
//	   0	 1	  2	  3    4	5	 6	  7	   8	9	10	 11	  12   13	14	 15
/*0*/{0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
/*1*/ 0x32,0x30,0x2e,0x31,0x32,0x2e,0x32,0x30,0x31,0x39,0x20,0x76,0x32,0x2e,0x30,0x00};
uint16_t timer_counter=0;
uint16_t timer_update=0;
uint8_t ready_for_processing=0;
uint8_t ready_for_update=0;
/******************************************** Use functions *****************************************/
void unload_eeprom_data(void)
{
	char str_test[70]={'\0'};
	//eeprom_write_block((void*)&eeprom_test,(void*)(0),32);
	
	eeprom_modbus_adress = eeprom_read_byte((uint8_t*)(adr_mbadress));
	num_boot_Sys = eeprom_read_word((uint16_t*) (adr_boot_num));
	num_boot_Sys++;
	eeprom_write_word((uint16_t*) (adr_boot_num),num_boot_Sys);
	eeprom_num_req=eeprom_read_dword((uint32_t*) (adr_num_req));
	eeprom_num_err=eeprom_read_dword((uint32_t*) (adr_num_err));
	eeprom_read_block((void*)&eeprom_soft_version, (const void*)adr_version, 15);
}
void update_eeprom_MB(uint8_t *mb)
{
	eeprom_write_byte((uint8_t*)(adr_mbadress),*mb);
	eeprom_modbus_adress = *mb;
}
void update_eeprom_counters(uint32_t *req,uint32_t *err)
{	
	eeprom_write_dword((uint32_t*)(adr_num_err), *err);
	eeprom_write_dword((uint32_t*)(adr_num_req), *req);
}

extern uint8_t fn;
extern uint8_t in_crc_err;
extern uint8_t status;
extern void create_new_messages(uint8_t *B2_in_crc_err,uint8_t *B3_status,uint8_t *B4_fn,uint32_t *B4_amout_mb,uint32_t  *B8_amout_mb_error);

ISR(TIMER1_OVF_vect) //таймер обновления счетчиков в eeprom , один тик 8.388 сек
{
	timer_counter++;
	timer_update++;
	if (timer_counter==prescaller_counter)
	{
		ready_for_processing=1;
		timer_counter=0;
	}
	if (timer_update==prescaller_update_out_message)
	{
		ready_for_update=1;
		timer_update=0;

	}

	if (flag_modbus_message==1)
	{
		flag_modbus_message=0;
		LED_OFF
		LED_G_ON;
	}
			
	else if(flag_modbus_message==0)
	{
		LED_RG_ON
	}
	
	else if(flag_modbus_message==2)
	{
		LED_OFF
		LED_R_ON;
		flag_modbus_message=0;
	}
}