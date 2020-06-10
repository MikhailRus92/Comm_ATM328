/******************************************** Library ***********************************************/
#include "modbus.h"

/******************************************** Variables ********************************************/
uint8_t rx_data_mb=0,rx_start_read_mb=0,rx_completed_mb=0,rx_index_mb=0;
char rx_buffer_mb[rx_buffer_length_mb]="";
char tx_buffer_mb[50];
uint8_t in=0;
char temp[10]={'\0'};
uint8_t flag_modbus_message=0;
/******************************************** MB Variables ******************************************/
volatile unsigned char BusState = 0; //������ ������ ���������
volatile uint16_t modbusTimer = 0;
volatile unsigned char rxbuffer[MaxFrameIndex+1];
volatile uint16_t DataPos = 0;
volatile unsigned char PacketTopIndex = 7;
/*********************************** internal_function_declaration********************************/
void transceiver_txen(void);
void transceiver_rxen(void);
/******************************************** Functions *****************************************/
void printStr_mb (char *str)
{
	uint16_t l = strlen(str);
	transceiver_txen();
	for (int i=0; i < l; i++)
	{
		while (!( UCSR0A & (1<<UDRE0))) {}
		UDR0=str[i];
	}
	_delay_us(500);
	transceiver_rxen();
}
void printStr_len_mb (char *str, uint8_t l)
{
	transceiver_txen();
	for (int i=0; i < l; i++)
	{
		while (!( UCSR0A & (1<<UDRE0))) {}
		UDR0=str[i];
	}
	_delay_ms(1);
	transceiver_rxen();
}
/******************************************** MB Functions ***************************************/
//������� �������������
uint8_t modbusGetBusState(void) //������ ������� ���������
{
	return BusState;
}
void transceiver_txen(void)//+
{
	PORT_MB|=(1<<PIN_MB_RTS); //�������� ����� �������� ��������� rts ->1
} 
void transceiver_rxen(void)//+
{
	PORT_MB&=~(1<<PIN_MB_RTS); //�������� ������������� ����� rts ->0
}
//������� ���������
uint8_t crc16(volatile uint8_t *ptrToArray,uint8_t inputSize) //����������� CRC ��������
{
	uint16_t out=0xffff;
	uint16_t carry;
	unsigned char n;
	inputSize++;
	for (int l=0; l<inputSize; l++) {
		out ^= ptrToArray[l];
		for (n = 0; n < 8; n++) {
			carry = out & 1;
			out >>= 1;
			if (carry) out ^= 0xA001;
		}
	}
	if ((ptrToArray[inputSize]==out%256) && (ptrToArray[inputSize+1]==out/256)) //��������
	{
		return 1;

	} 
		
	else 
	{
		ptrToArray[inputSize]=out%256; //������������ Lo
		ptrToArray[inputSize+1]=out/256; //������������ Hi
		

		return 0;
	}
}
void modbusReset(void) //����� ������� � ���� ��������
{
	BusState=(1<<TimerActive);
	modbusTimer=0;
}
void modbusTickTimer(void)
{
	if (BusState&(1<<TimerActive)) //���� uart ���������������
	{
		modbusTimer++; //����� ������ ���������� �� ��������� � ���������� �� ������� � ��������� ���� �� ������� �����
		
		if (BusState&(1<<Receiving)) //�� ��������� � ��������� ������ ������
		{
			if ((modbusTimer==modbusInterCharTimeout))
			{
				//���� ����� ������� ������ ������ n ������������
				BusState|=(1<<GapDetected); //��������� ������
			}
			else if ((modbusTimer==modbusInterFrameDelayReceiveEnd))
			{ //���� ������ n ������������
				

				BusState=(1<<ReceiveCompleted); //������� �����������, ���������� ����, ��� ����� ������������� ���������
				/*transceiver_txen();
				transceiver_rxen();
				sprintf(tx_buffer_mb,"%d",in);
				printStr_mb(tx_buffer_mb);
				printStr_len_mb(temp,in);
				in=0;*/
				
				eeprom_num_req++;
			
				if (rxbuffer[0]==eeprom_modbus_adress)
				{
					if (crc16(rxbuffer,DataPos-3))
					{
						flag_modbus_message=1;		
					}
					else
					{
						eeprom_num_err++;
						modbusReset();//����� ������� � ���� ��������
					}				
				}
				else
				{
					modbusReset();//����� ������� � ���� ��������
				}
				
				
			}
		}
		else if (modbusTimer==modbusInterFrameDelayReceiveStart) BusState|=(1<<BusTimedOut);  //���� ����� ������� ������� ��������� n ������������ , �.� 1,6384 �� , ���������� ���� ���������� � ������
	}
}

void modbusSendMessage(unsigned char packtop)
{
	PacketTopIndex=packtop+2;
	crc16(rxbuffer,packtop); //��������� ����������� �����
	BusState|=(1<<TransmitRequested); //��������� ������ ���������, ���������� � ��������
	DataPos=0; //������������ ��������� ����
	
	transceiver_txen(); //�������� ����� �������� ��������� rts ->1
	
	//LAMP1_OFF
	//TIMSK1&=~(1<<TOIE1);
	UART_CONTROL|=(1<<UART_UDRIE); //��������� ���������� UART_TRANSMIT_INTERRUPT
	BusState&=~(1<<ReceiveCompleted); //������� ������ ���������, ����� ��������� ��������
}
void modbusSendException(unsigned char exceptionCode)
{
	rxbuffer[1]|=(1<<7); //������������� ���� ������� ����������
	rxbuffer[2]=exceptionCode; //Exceptioncode
	modbusSendMessage(2);
}
//������� ������
uint16_t modbusRequestedAmount(void)
{
	return (rxbuffer[5]|(rxbuffer[4]<<8)); //��������� ������������ �����, ��� ����� ���� �������, 4 ���� �������
}
uint16_t modbusRequestedAddress(void)
{
	return (rxbuffer[3]|(rxbuffer[2]<<8)); //��������� ������������ �����, ��� ������ ���� �������, 2 ���� �������
}
void intToModbusRegister(volatile uint16_t *inreg, volatile uint8_t *outreg, uint8_t amount)
{
	for (uint8_t x=0; x<amount; x++)
	{
		*(outreg+x*2) = (uint8_t)(*(inreg+x) >> 8);
		*(outreg+1+x*2) = (uint8_t)(*(inreg+x));
	}
}
void modbusExchangeRegisters(volatile uint16_t *ptrToInArray, uint16_t startAddress, uint16_t size)
{
	//LED_B_ON;
	uint16_t requestedAmount = modbusRequestedAmount(); //�������� ������������ ����� � ����������� ������ , ������� ����� ���������/��������
	uint16_t requestedAdr = modbusRequestedAddress(); //�������� ������������ ����� � ������� ������ , � �������� ����� �������� ������/������
	if ((requestedAdr>=startAddress) && ((startAddress+size)>=(requestedAmount+requestedAdr)))
	{//���� ��������� ��������� ����� ������ �������� � ��������� �����+������ ������ ��� ����� ����� �������
		
		if (rxbuffer[1]==fcReadHoldingRegisters) //���� ��� ������� 3
		{
			if ((requestedAmount*2)<=(MaxFrameIndex-4)) //���� ��������� ���������� ���������*2(1 ��� 2 �����)  ������ 255 - 4 ����� (�������,�����,crc*2)
			{
				rxbuffer[2]=(unsigned char)(requestedAmount*2);
				intToModbusRegister(ptrToInArray+(unsigned char)(requestedAdr-startAddress),rxbuffer+3,requestedAmount);
				modbusSendMessage(2+rxbuffer[2]);
			}
			else 
			{
				modbusSendException(ecIllegalDataValue);
				eeprom_num_err++;
			}
		}
	}
	else
	{
		modbusSendException(ecIllegalDataValue);eeprom_num_err++;;
	}
}
void modbusGet(void) { //������� �������� ���������� �������
	if (modbusGetBusState() & (1<<ReceiveCompleted)) //���� ������ ��������� "��������� ������ ���������"
	{
		//LED_OFF
		
		//eeprom_num_req++;
		switch(rxbuffer[1]) { 
			case fcReadHoldingRegisters: { // (0x03) � ������ �������� �� ���������� ��������� �������� (Read Holding Registers).
			modbusExchangeRegisters(holdingRegisters,0,16);} 
			break;
			case fcReportSlaveID: { //(0x11) � ������ ���������� �� ���������� (Report Slave ID)
			}
			break;
			default: {
				modbusSendException(ecIllegalFunction);
			}
			break;
		}
	}	
			
}
/******************************************** Interrupt ********************************************/
/*ISR(USART_RX_vect)//bluetooth
{
	rx_data_mb=UDR0;
	if ((rx_data_mb==0x3e) && (rx_start_read_mb==1))
	{
		rx_start_read_mb=0;
		rx_completed_mb=1;
	}
	if (rx_start_read_mb==1)
	{
		rx_buffer_mb[rx_index_mb]=rx_data_mb;
		rx_index_mb++;
	}
	if ((rx_data_mb == 0x3c)&&(rx_start_read_mb==0)&&(rx_completed_mb==0)) {rx_start_read_mb=1;rx_index_mb=0;}

}*/
ISR(TIMER0_OVF_vect)//����������� ����������  �.� 8���/256 =31250 ��� � �������, 256 - ������ ������� 0,032 ��
{
	modbusTickTimer();
}
ISR(USART_UDRE_vect)
{
	BusState&=~(1<<TransmitRequested);
	BusState|=(1<<Transmitting);
	UART_DATA=rxbuffer[DataPos];
	DataPos++;
	if (DataPos==(PacketTopIndex+1)) {
		UART_CONTROL&=~(1<<UART_UDRIE);
	}
}
ISR(USART_TX_vect)
{
	transceiver_rxen(); //�������� ������������� ����� rts ->0
	modbusReset();  //����� ������� � ���� ��������
}
ISR(USART_RX_vect)
{
	//BusState|=(1<<adc_stop); //��� ��������
	unsigned char data;
	data = UART_DATA;
	//temp[in]=data;
	//in++;
	modbusTimer=0; //����� �������, ����� ������ ������ ������
	if (!(BusState & (1<<ReceiveCompleted)) && !(BusState & (1<<TransmitRequested)) && !(BusState & (1<<Transmitting)) && (BusState & (1<<Receiving)) && !(BusState & (1<<BusTimedOut))) //���� ������� ����� ������ �.� ������ ������ Receiving
	{
		if (DataPos>MaxFrameIndex) modbusReset(); //���� ����� ���������� ������� ������ ������������ �.� 255 �� ����� ������� � ���� ��������
		else
		{
			rxbuffer[DataPos]=data; //���� ������ ������� � �����, �� ��������� ��������� ����
			DataPos++; //�������������� ������ ����������� �����
		}
	}
	else if (!(BusState & (1<<ReceiveCompleted)) && !(BusState & (1<<TransmitRequested)) && !(BusState & (1<<Transmitting)) && !(BusState & (1<<Receiving)) && (BusState & (1<<BusTimedOut))) // ��� ��� ������ ���� � �������
	{
		
		rxbuffer[0]=data; //��������� ����� � ������� ������

		BusState=((1<<Receiving)|(1<<TimerActive)); //������������� ���� ��������� ������ � ��������� ������� //������� ���� ��������
		DataPos=1; //�������������� ������ ����������� �����
	}
}