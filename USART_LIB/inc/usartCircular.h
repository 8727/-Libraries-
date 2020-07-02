#ifndef USARTCIRCULAR_H_
#define USARTCIRCULAR_H_


#define MYUART huart1                     // задефайнить USART
#define UART_BUFFER_SIZE              128 // указать размер приёмного буфера

uint16_t uartAvailable(void);
uint8_t uartRead(void);
void usartClearBuff(void);

#endif /* USARTCIRCULAR_H_ */

















