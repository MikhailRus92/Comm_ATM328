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
volatile unsigned char BusState = 0; //Статус работы протокола
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
//функции инициализации
uint8_t modbusGetBusState(void) //запрос статуса протокола
{
	return BusState;
}
void transceiver_txen(void)//+
{
	PORT_MB|=(1<<PIN_MB_RTS); //включаем режим передачи сообщения rts ->1
} 
void transceiver_rxen(void)//+
{
	PORT_MB&=~(1<<PIN_MB_RTS); //включаем прослушивание порта rts ->0
}
//функции сервисные
uint8_t crc16(volatile uint8_t *ptrToArray,uint8_t inputSize) //Стандартный CRC Алгоритм
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
	if ((ptrToArray[inputSize]==out%256) && (ptrToArray[inputSize+1]==out/256)) //проверка
	{
		return 1;

	} 
		
	else 
	{
		ptrToArray[inputSize]=out%256; //присоединяем Lo
		ptrToArray[inputSize+1]=out/256; //присоединяем Hi
		

		return 0;
	}
}
void modbusReset(void) //сброс таймера и всех статусов
{
	BusState=(1<<TimerActive);
	modbusTimer=0;
}
void modbusTickTimer(void)
{
	if (BusState&(1<<TimerActive)) //если uart инициализирован
	{
		modbusTimer++; //Когда таймер переполнен мы переходим в прерывание по таймеру и проверяем есть ли команда извне
		
		if (BusState&(1<<Receiving)) //Мы находимся в состояние приема данных
		{
			if ((modbusTimer==modbusInterCharTimeout))
			{
				//Если между байтами прошло больше n переполнений
				BusState|=(1<<GapDetected); //обнаружен пробел
			}
			else if ((modbusTimer==modbusInterFrameDelayReceiveEnd))
			{ //если прошло n переполнений
				

				BusState=(1<<ReceiveCompleted); //посылка закончилась, выставляем флаг, что можно анализировать сообщение
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
						modbusReset();//сброс таймера и всех статусов
					}				
				}
				else
				{
					modbusReset();//сброс таймера и всех статусов
				}
				
				
			}
		}
		else if (modbusTimer==modbusInterFrameDelayReceiveStart) BusState|=(1<<BusTimedOut);  //если после запуска таймера произошло n переполнений , т.е 1,6384 мс , выставляем флаг готовности к чтению
	}
}

void modbusSendMessage(unsigned char packtop)
{
	PacketTopIndex=packtop+2;
	crc16(rxbuffer,packtop); //добавляем контрольную сумму
	BusState|=(1<<TransmitRequested); //добавляем статус протокола, готовность к отправке
	DataPos=0; //Устанавливам стартовый байт
	
	transceiver_txen(); //включаем режим передачи сообщения rts ->1
	
	//LAMP1_OFF
	//TIMSK1&=~(1<<TOIE1);
	UART_CONTROL|=(1<<UART_UDRIE); //запускаем прерывание UART_TRANSMIT_INTERRUPT
	BusState&=~(1<<ReceiveCompleted); //Удаляем статус протокола, прием сообщение завершен
}
void modbusSendException(unsigned char exceptionCode)
{
	rxbuffer[1]|=(1<<7); //устанавливаем флаг функции исключение
	rxbuffer[2]=exceptionCode; //Exceptioncode
	modbusSendMessage(2);
}
//функции приема
uint16_t modbusRequestedAmount(void)
{
	return (rxbuffer[5]|(rxbuffer[4]<<8)); //формируем двухбайтовое число, где пятый байт младший, 4 байт старший
}
uint16_t modbusRequestedAddress(void)
{
	return (rxbuffer[3]|(rxbuffer[2]<<8)); //формируем двухбайтовое число, где третий байт младший, 2 байт старший
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
	uint16_t requestedAmount = modbusRequestedAmount(); //приходит двухбайтовое число с количеством данных , которые нужно прочитать/записать
	uint16_t requestedAdr = modbusRequestedAddress(); //приходит двухбайтовое число с номером адреса , с которого нужно начинать чтение/запись
	if ((requestedAdr>=startAddress) && ((startAddress+size)>=(requestedAmount+requestedAdr)))
	{//если требуемый начальный адрес больше нулевого и стартовый адрес+размер больше или равно сумме запроса
		
		if (rxbuffer[1]==fcReadHoldingRegisters) //если код функции 3
		{
			if ((requestedAmount*2)<=(MaxFrameIndex-4)) //если требуемое количество регистров*2(1 рег 2 байта)  меньше 255 - 4 байта (функция,адрес,crc*2)
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
void modbusGet(void) { //функция проверки полученной команды
	if (modbusGetBusState() & (1<<ReceiveCompleted)) //если статус протокола "Получение пакета завершено"
	{
		//LED_OFF
		
		//eeprom_num_req++;
		switch(rxbuffer[1]) { 
			case fcReadHoldingRegisters: { // (0x03) — чтение значений из нескольких регистров хранения (Read Holding Registers).
			modbusExchangeRegisters(holdingRegisters,0,16);} 
			break;
			case fcReportSlaveID: { //(0x11) — Чтение информации об устройстве (Report Slave ID)
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
ISR(TIMER0_OVF_vect)//Прерыввание вызывается  т.е 8Мгц/256 =31250 раз в секунду, 256 - размер таймера 0,032 мс
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
	transceiver_rxen(); //включаем прослушивание порта rts ->0
	modbusReset();  //сброс таймера и всех статусов
}
ISR(USART_RX_vect)
{
	//BusState|=(1<<adc_stop); //тут внимание
	unsigned char data;
	data = UART_DATA;
	//temp[in]=data;
	//in++;
	modbusTimer=0; //Сброс таймера, перед каждым байтом данных
	if (!(BusState & (1<<ReceiveCompleted)) && !(BusState & (1<<TransmitRequested)) && !(BusState & (1<<Transmitting)) && (BusState & (1<<Receiving)) && !(BusState & (1<<BusTimedOut))) //если начался прием данных т.е статус модбас Receiving
	{
		if (DataPos>MaxFrameIndex) modbusReset(); //если объем полученной посылки больше разрешенного т.е 255 то сброс таймера и всех статусов
		else
		{
			rxbuffer[DataPos]=data; //если размер посылки в норме, то сохраняем следующий байт
			DataPos++; //инкриментируем индекс полученного байта
		}
	}
	else if (!(BusState & (1<<ReceiveCompleted)) && !(BusState & (1<<TransmitRequested)) && !(BusState & (1<<Transmitting)) && !(BusState & (1<<Receiving)) && (BusState & (1<<BusTimedOut))) // ели это первый байт с адресом
	{
		
		rxbuffer[0]=data; //сохраняем адрес в входной массив

		BusState=((1<<Receiving)|(1<<TimerActive)); //устанавливаем биты получения данных и активного таймера //удаляем флаг таймаута
		DataPos=1; //инкриментируем индекс полученного байта
	}
}