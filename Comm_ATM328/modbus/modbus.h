/******************************************** Library ***********************************************/
#include "init.h"

/******************************************** Difinition ********************************************/
#define rx_buffer_length_mb 20
#define MaxFrameIndex 255 //������������ ������ ������

#define modbusInterFrameDelayReceiveStart 59//������������ ������� 3.5 (����) * 11(���) * 26,041 = 1,0026 ��; ������ ������������� 1 ��� � 0.256��; x = 1,0026 / 0.256 =
#define modbusInterFrameDelayReceiveEnd 59//���� ������� ������ 19200 ���������� �������� >1.75 � <0.75
#define modbusInterCharTimeout 17

//�������� ������� �� ��������� modbus rtu
#define fcReadHoldingRegisters 3 //������ �������� �� ���������� ��������� �������� (Read Holding Registers).
#define fcReportSlaveID 17 //������ ���������� �� ���������� (Report Slave ID)
#define ecIllegalFunction 1  //�������� ��� ������� �� ����� ���� ���������.
#define ecIllegalDataAddress 2 //����� ������, ��������� � �������, ����������.
#define ecIllegalDataValue 3 //��������, ������������ � ���� ������ �������, �������� ������������ ���������.
#define ecSlaveDeviceFailure 4 //������������������� ������ ����� �����, ���� ������� ���������� �������� ��������� ������������� ��������.
#define ecAcknowledge 5 //������� ���������� ������� ������ � ������������ ���, �� ��� ������� ����� �������. ���� ����� ������������ ������� ���������� �� ��������� ������ ����-����.
#define ecSlaveDeviceBusy 6 //������� ���������� ������ ���������� �������. ������� ���������� ������ ��������� ��������� �����, ����� ������� �����������.
#define ecNegativeAcknowledge 7 //������� ���������� �� ����� ��������� ����������� �������, �������� � �������. ���� ��� ������������ ��� ����������� ������������ �������, ������������� ������� � �������� 13 ��� 14. ������� ���������� ������ ��������� ��������������� ���������� ��� ���������� �� ������� �� ��������
#define ecMemoryParityError 8 //������� ���������� ��� ������ ����������� ������ ���������� ������ ��������. ������� ���������� ����� ��������� ������, �� ������ � ����� ������� ��������� ������.

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
extern uint8_t crc16(volatile uint8_t *ptrToArray,uint8_t inputSize); //����������� CRC ��������

