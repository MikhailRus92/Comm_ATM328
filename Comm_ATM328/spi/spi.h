/******************************************** Library ***********************************************/
#include "init.h"
/******************************************** Difinition ********************************************/
#define adr_boot_num 1
/******************************************** Used functions *****************************************/
extern void printStr_mb(char *str);
extern void printStr_len_mb (char *str, uint8_t l);
void create_new_messages(uint8_t *B2_in_crc_err,uint8_t *B3_status,uint8_t *B4_fn,uint32_t *B4_amout_mb,uint32_t  *B8_amout_mb_error);
uint8_t check_in_crc16(char *indata, uint8_t *crc_err);
void convert_incoming_message(char *indata,float *ai_array, uint8_t *MB,uint8_t *DO_st, uint8_t *st);
extern void update_eeprom_MB(uint8_t *mb);
extern void update_eeprom_counters(uint32_t *req,uint32_t *err);
/******************************************** TypeDef ********************************************/
union FloatType //универсальный тип
{
	float floatVal;
	unsigned char  buf[4];
	uint16_t uint16_buf[2];
};
union IntType
{
	uint32_t intVal; 
	unsigned char  buf[4];
};
union IntType uint_spi1;
union IntType uint_spi2;
union FloatType float_value_spi; //переменная для передачи float слэйву