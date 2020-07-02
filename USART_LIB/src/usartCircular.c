#include "usartCircular.h"

extern UART_HandleTypeDef MYUART;

volatile uint16_t rxBufferHead = 0x00;
volatile uint16_t rxBufferTail = 0x00;
uint8_t rxBuffer[UART_BUFFER_SIZE] = {0x00,};

void usartClearBuff(void){
  __HAL_UART_DISABLE_IT(&MYUART, UART_IT_RXNE);
  rxBufferHead = 0;
  rxBufferTail = 0;
  __HAL_UART_ENABLE_IT(&MYUART, UART_IT_RXNE);
}

uint8_t uartAvailable(void){
  return ((uint8_t)(UART_BUFFER_SIZE + rxBufferHead - rxBufferTail)) % UART_BUFFER_SIZE;
}

uint8_t uartRead(void){
  if(rxBufferHead == rxBufferTail){
    return 0;
  }else{
    uint8_t c = rx_buffer[rxBufferTail];
    rxBufferTail = (uint16_t)(rxBufferTail + 1) % UART_BUFFER_SIZE;
    return c;
	}
}







