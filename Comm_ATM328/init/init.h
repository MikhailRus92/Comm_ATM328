/******************************************** F_CPU ********************************************/
#define F_CPU 8000000UL//8000000UL
/***************************************** SYSLibrary ***********************************************/
#ifndef INC_LIST
#define INC_LIST
#include <avr/pgmspace.h> //пространство типов данных
#include <string.h> //библиотека работы со строками
#include <stdio.h> // библиотека форматировани€ текста
#include <stdlib.h> //преобразование типов
//#include <inttypes.h> вроде не нужно
#include <avr/io.h> //GPIO
#include <avr/interrupt.h> // векторы прерывани€
#include <avr/wdt.h> //библиотека WDT таймера
#include <util/delay.h> //библиотека задержек
#include <avr/eeprom.h> //библиотека энергонезависимой пам€ти
#endif
/**************************************** Difinition periphery **************************************/
#ifndef PER_LIST
#define PER_LIST
//SPI / Comunication MCU-MCU
#define PORT_SPI PORTB
#define PIN_MOSI PINB3
#define PIN_MISO PINB4
#define PIN_SCK PINB5
#define PIN_CS PINB2
//////
#define PORT_COM PORTD
#define PIN_MOSI_COM PIND2
#define PIN_MISO_COM PIND3
//LED & DO & Status led
#define PORT_LED_DO PORTC
#define PIN_LED_R PINC3
#define PIN_LED_G PINC4
#define PIN_LED_B PINC5
//USART Modbus
//#define BAUD_MB 9600L //скорость передачи
#define BAUD_MB 38400L //38400



#define UART_STATUS   UCSR0A
#define UART_CONTROL  UCSR0B
#define UART_DATA     UDR0
#define UART_UDRIE    UDRIE0
#define UCSRC UCSR0C
#define RXCIE RXCIE0
#define TXCIE TXCIE0
#define RXEN RXEN0
#define TXEN TXEN0
#define U2X U2X0
#define UBRRH UBRR0H
#define UCSZ0 UCSZ00
#define UBRRL UBRR0L

#define UBRR_MB (F_CPU / 8 / BAUD_MB ) -1 //записываем в решистр
#define PORT_MB PORTD
#define PIN_MB_RXD PIND0
#define PIN_MB_TXD PIND1
#define PIN_MB_RTS PIND4


#endif

#define BusTimedOut 0  //‘лаг готовности устройства
#define Receiving 1  //‘лаг получени€ данных
#define Transmitting 2 //‘лаг передачи данных
#define ReceiveCompleted 3 //‘лаг завершени€ обработки вход€щих данных
#define TransmitRequested 4 //‘лаг запроса на передачу
#define TimerActive 5 //“аймер вкл
#define GapDetected 6 //ќбнаружен пробел

#define STOP_TX PORT_COM|=(1<<PIN_MISO_COM)
#define RESET_STOP_TX PORT_COM&=~(1<<PIN_MISO_COM)

//led RGB
#define LED_G_OFF PORT_LED_DO|=(1<<PIN_LED_R)
#define LED_B_OFF PORT_LED_DO|=(1<<PIN_LED_G)
#define LED_R_OFF PORT_LED_DO|=(1<<PIN_LED_B)
#define LED_G_ON PORT_LED_DO&=~(1<<PIN_LED_R)
#define LED_B_ON PORT_LED_DO&=~(1<<PIN_LED_G)
#define LED_R_ON PORT_LED_DO&=~(1<<PIN_LED_B)
//led RGB
#define LED_RG_ON LED_R_ON;LED_G_ON;LED_B_OFF;
#define LED_BR_ON LED_B_ON;LED_R_ON;LED_G_OFF;
#define LED_BG_ON LED_G_ON;LED_B_ON;LED_R_OFF;
#define LED_OFF LED_R_OFF;LED_G_OFF;LED_B_OFF;

/******************************************** Difinition ********************************************/
#define outnum_bytes_SPI 15
#define innum_bytes_SPI 39
/******************************************** Use functions *****************************************/
void init_uart_mb(void);
void SPI_Init_Slave(void);
void timer_counter_init(void);
void timer_modbus_check(void);
extern void transceiver_rxen(void);
extern void modbusInit(void);
void gpio_init(void);
/******************************************** Variables ********************************************/
uint8_t eeprom_modbus_adress;
uint16_t num_boot_Sys;
char eeprom_soft_version[16];
uint32_t eeprom_num_req;
uint32_t eeprom_num_err;
extern volatile unsigned char BusState;
extern uint16_t holdingRegisters[16];