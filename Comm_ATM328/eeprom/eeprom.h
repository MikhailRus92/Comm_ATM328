/******************************************** Library ************************************************/
#include "init.h"
/******************************************** Use functions ******************************************/
void unload_eeprom_data(void);
extern void printStr_mb(char *str);
void update_eeprom_MB(uint8_t *mb);
extern void update_eeprom_counters(uint32_t *req,uint32_t *err);
/******************************************** Difinition *********************************************/
#define adr_mbadress 0
#define adr_boot_num 1
#define adr_num_req 3
#define adr_num_err 7
#define adr_version 16
#define prescaller_counter 400 //2575   = 21600 сек - 6 часов
#define prescaller_update_out_message 15
/******************************************** Variables **********************************************/
extern uint8_t flag_modbus_message; 