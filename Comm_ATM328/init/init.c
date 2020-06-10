/******************************************** Library ***********************************************/
#include "init.h"  //���������� ��������
/******************************************** Variables *******************************************/
uint8_t error=0;  //00000001 ������ ����������� �������� //00000010 ������ ����������� ��������
uint8_t warning=0;
uint8_t status=0;
/******************************************** Functions *****************************************/
void init_uart_mb(void)
{
	UBRR0H=(unsigned char)((UBRR_MB) >> 8);
	UBRR0L=(unsigned char) UBRR_MB;
	UCSR0A = (1<<U2X0);
	UCSR0B=(1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);
}
void SPI_Init_Slave(void)
{
	DDRB&= ~(1<<PIN_SCK) | ~(1<<PIN_CS) |~(1<<PIN_MOSI); //����������� ������ SCK,MOSI,SS �� ����
	DDRB|=(1<<PIN_MISO);// ����������� ����� MISO �� �����
	
	SPCR=0;//�������� ������� SPCR
	SPSR=0;//�������� ������� SPSR

	SPCR |= (1<<SPR1)|(1<<SPIE);//Fosc=F/16,���������� ��������
	SPCR |=(1<<SPE);//�������� SPI
	
	//DDRD &= ~(1<<PIND3); //MOSI �������  PD3(1) << 25 ����  int1
	//DDRD |= (1<<PIND2); //MISO ������� PD2(32) >> 26 �����
	//EICRA |=(1<<ISC10)/*|(1<<ISC11)*/;
	//EIMSK |= (1<<INT1);
}
void timer_counter_init(void) //16 ������ ������
{
	TCCR1B|=(1<<CS12)|(1<<CS10); //�������� 011 8���/1024/65536 1 ��� � 8.388 ���. 
	TIMSK1|=(1<<TOIE1); //��������� ���������� �� ������������ ������� 1
}
void timer_modbus_check(void) //
{
	TCCR0B|=(1<<CS00);//�������� 1   ������� 0
	TIMSK0|=(1<<TOIE0); //��������� ���������� �� ������������ ������� 0
}
void modbusInit(void)
{
	UBRRH = (unsigned char)((UBRR_MB) >> 8);  // �������� �������� 38400
	UBRRL = (unsigned char) UBRR_MB; //
	UART_STATUS = (1<<U2X0); // ��������� ��������
	UCSRC = (3<<UCSZ0); //8 ��� �������
	UART_CONTROL = (1<<TXCIE)|(1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); // USART ����������
	DDRD|=(1<<PIN_MB_RTS); // ��������� ����� RTS ������
	transceiver_rxen(); //�������� ������������� ����� rts ->0
	BusState=(1<<TimerActive);  //������ ��������� ��������
	timer_modbus_check();
}
void Modbus_stop(void)
{
	UART_CONTROL &=~ ((1<<TXCIE)|(1<<RXCIE)|(1<<RXEN)|(1<<TXEN));
	TIMSK0&=~(1<<TOIE0);
}
void gpio_init(void)
{
	DDRC |= (1<<PIN_LED_B)|(1<<PIN_LED_R)|(1<<PIN_LED_G); //���������� ������������ ���������
	
	DDRD &= ~(1<<PIN_MOSI_COM); //MOSI
	DDRD |= (1<<PIN_MISO_COM); //MISO
	EICRA |=(1<<ISC00)|(1<<ISC01); //���������� �����
	EIMSK |= (1<<INT0);
	RESET_STOP_TX;
	
}