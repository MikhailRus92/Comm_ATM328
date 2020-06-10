/*
 * Title:Главный заголовочный файл проекта
 * Comm_ATM328.c
 * Created: 29.11.2019
 * Author : Rusaleev MB
 */ 
/******************************************** SelfLibrary *************************************************/
#include "spi.h" //SPI библиотека
#include "gpio.h" //Библиотека обработка прерываний PCINT
//#include "it.h" //прерывания
#include "modbus.h" //modbus
#include "init.h" //инициализация
#include "eeprom.h"
/******************************************** Extern Variables *********************************************/
extern uint8_t rx_completed_spi;
extern char incoming_message[innum_bytes_SPI];
extern char outcoming_message[outnum_bytes_SPI];
extern uint8_t ready_for_processing;
extern uint8_t ready_for_update;
extern uint8_t SPI_STAT;
extern uint16_t counter_SPI;
/******************************************** Variables *****************************************************/
float ai_float[8];
uint8_t in_modbus_adress;
uint8_t DO_status;
uint8_t in_status;
uint16_t holdingRegisters[16] = {00}; //регистры для обмена данными по модбас
	
/******************************************** Definition ****************************************************/
