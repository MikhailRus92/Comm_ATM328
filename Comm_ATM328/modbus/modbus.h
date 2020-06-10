/******************************************** Library ***********************************************/
#include "init.h"

/******************************************** Difinition ********************************************/
#define rx_buffer_length_mb 20
#define MaxFrameIndex 255 //максимальный размер пакета

#define modbusInterFrameDelayReceiveStart 59//длительность посылки 3.5 (симв) * 11(бит) * 26,041 = 1,0026 мс; таймер переполняется 1 раз в 0.256мс; x = 1,0026 / 0.256 =
#define modbusInterFrameDelayReceiveEnd 59//если битрейт больше 19200 используем таймауты >1.75 и <0.75
#define modbusInterCharTimeout 17

//Описание функций из протокола modbus rtu
#define fcReadHoldingRegisters 3 //чтение значений из нескольких регистров хранения (Read Holding Registers).
#define fcReportSlaveID 17 //Чтение информации об устройстве (Report Slave ID)
#define ecIllegalFunction 1  //Принятый код функции не может быть обработан.
#define ecIllegalDataAddress 2 //Адрес данных, указанный в запросе, недоступен.
#define ecIllegalDataValue 3 //Значение, содержащееся в поле данных запроса, является недопустимой величиной.
#define ecSlaveDeviceFailure 4 //Невосстанавливаемая ошибка имела место, пока ведомое устройство пыталось выполнить затребованное действие.
#define ecAcknowledge 5 //Ведомое устройство приняло запрос и обрабатывает его, но это требует много времени. Этот ответ предохраняет ведущее устройство от генерации ошибки тайм-аута.
#define ecSlaveDeviceBusy 6 //Ведомое устройство занято обработкой команды. Ведущее устройство должно повторить сообщение позже, когда ведомое освободится.
#define ecNegativeAcknowledge 7 //Ведомое устройство не может выполнить программную функцию, заданную в запросе. Этот код возвращается для неуспешного программного запроса, использующего функции с номерами 13 или 14. Ведущее устройство должно запросить диагностическую информацию или информацию об ошибках от ведомого
#define ecMemoryParityError 8 //Ведомое устройство при чтении расширенной памяти обнаружило ошибку паритета. Ведущее устройство может повторить запрос, но обычно в таких случаях требуется ремонт.

/******************************************** Variables ********************************************/
volatile unsigned char rxbuffer[MaxFrameIndex+1];
extern uint16_t holdingRegisters[16];
extern union FloatType float_value_mb;

/******************************************** Used functions *****************************************/
void printStr_mb (char* str);
void printStr_len_mb (char *str, uint8_t l);
/******************************************** Used MD functions *****************************************/
extern uint8_t modbusGetBusState(void);
extern void modbusGet(void);
extern uint8_t crc16(volatile uint8_t *ptrToArray,uint8_t inputSize); //Стандартный CRC Алгоритм

