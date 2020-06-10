/******************************************** Library ***********************************************/
#include "spi.h"
/******************************************** Variables ********************************************/
char incoming_message[innum_bytes_SPI]={'\0'};
char outcoming_message[outnum_bytes_SPI]={'\0'};
uint8_t rx_data_spi=0,rx_start_read_spi=0,rx_completed_spi=0,rx_index_spi=0;
uint8_t SPI_STAT=0;
uint16_t counter_SPI=0;
/******************************************** Use functions *****************************************/
ISR(SPI_STC_vect)
{	
	
	rx_data_spi = SPDR; 
	if ((rx_data_spi==0x3e) && (rx_start_read_spi==1) && (rx_index_spi==innum_bytes_SPI-2))
	{
		rx_start_read_spi=0;
		rx_completed_spi=1;
		SPI_STAT=1;
			/*	LED_OFF;
				LED_G_ON;*/
		
		
	}
	if (rx_index_spi>innum_bytes_SPI-2) //ошибка передачи
	{
		STOP_TX;
		rx_start_read_spi=0;
		/*LED_OFF;
		LED_R_ON;*/
		RESET_STOP_TX;
		SPI_STAT=99;
		
	}
	if (rx_start_read_spi==1)
	{
		incoming_message[rx_index_spi]=rx_data_spi;
		rx_index_spi++;
		counter_SPI++;
	}
	if ((rx_data_spi == 0x3c)&&(rx_start_read_spi==0)&&(rx_completed_spi==0)) 
	{
		rx_start_read_spi=1;
		rx_index_spi=0;
	}
		
	if (rx_start_read_spi==1)
	{
		if (rx_index_spi<outnum_bytes_SPI)
		{
			SPDR = outcoming_message[rx_index_spi];
		}
		else SPDR = 0xAA;
	}
	else SPDR = 0xFF;	
}
uint8_t check_in_crc16(char *indata, uint8_t *crc_err)
{
	uint16_t in_crc16=0;
	//char str_temp[10]={'\0'};
	for (int i=0;i<innum_bytes_SPI-4;i++) //из сообщения убраны первый и последний байт, ВНИМАНИЕ!
	{
		in_crc16=in_crc16+indata[i];
	}
	if (in_crc16 == ((indata[innum_bytes_SPI-4]<<8)|indata[innum_bytes_SPI-3])) //из сообщения убраны первый и последний байт, ВНИМАНИЕ!
	{
		/*printStr_mb("CRC->OK");*/return(1);
		
	}
	else {/*printStr_mb("CRC->ERR");*/*crc_err = *crc_err + 1; 		
		STOP_TX;
		
		rx_start_read_spi=0;
		RESET_STOP_TX;return(0);}

	//sprintf(str_temp,"%d",*crc_err);
	//printStr_mb(str_temp);
	//printStr_mb(indata);
	
}
void create_new_messages(uint8_t *B2_in_crc_err,uint8_t *B3_status,uint8_t *B4_fn,uint32_t *B4_amout_mb,uint32_t  *B8_amout_mb_error)
{
	uint_spi1.intVal = *B4_amout_mb;
	uint_spi2.intVal = *B8_amout_mb_error;
	uint16_t out_crc16=0;
	
	//char str_temp[30]={'\0'};
		
	memset(outcoming_message,'\0',outnum_bytes_SPI);
	
	sprintf(outcoming_message,"<%c%c%c%c%c%c%c%c%c%c%c%c%c>",*B2_in_crc_err,*B3_status,*B4_fn,uint_spi1.buf[0],uint_spi1.buf[1],uint_spi1.buf[2],uint_spi1.buf[3], uint_spi2.buf[0],uint_spi2.buf[1],uint_spi2.buf[2],uint_spi2.buf[3],0xFF,0xFF);

	for (uint8_t i=1;i<outnum_bytes_SPI-3;i++)
	{
		out_crc16=out_crc16+outcoming_message[i];
	}
		
	//sprintf(str_temp,"eeprom_mb-4 %d \r\n",eeprom_modbus_adress);

	outcoming_message[outnum_bytes_SPI-3] = (out_crc16 >>8); //выставляем первый байт crc16 исходящего сообщения
	outcoming_message[outnum_bytes_SPI-2] = (out_crc16 & 0xFF); //выставляем второй байт crc16 исходящего сообщения
}
void convert_incoming_message(char *indata,float *ai_array, uint8_t *MB,uint8_t *DO_st, uint8_t *st)
{
	//char str_temp[40]={'\0'};
	*MB =  (indata[0]);
	*DO_st = (indata[1]);
	*st = (indata[2]);
	for (int i = 0;i<8;i++)
	{
		memcpy(&float_value_spi.buf[0],&indata[3+i*4],4);
		ai_array[i]=float_value_spi.floatVal;
		memcpy(&holdingRegisters[2*i],float_value_spi.uint16_buf,4); //сохраняем мин значение канала	
	}
	
	/*
	sprintf(str_temp,"incoming_MB %d\r\n do_st-%d\r\n",*MB,*DO_st);
	printStr_mb(str_temp);
	*/
	if (*st==0x3F)
	{
		eeprom_num_req=0;
		eeprom_num_err=0;
		update_eeprom_counters(&eeprom_num_req,&eeprom_num_err);
		eeprom_write_word((uint16_t*) (adr_boot_num),1);
		
	}
	if ((*MB)!=eeprom_modbus_adress)
	{
		//printStr_mb("ADRESS has been changed");
		update_eeprom_MB(MB);
	}
	
}
 