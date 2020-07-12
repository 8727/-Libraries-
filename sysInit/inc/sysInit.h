#ifndef SYSINIT_H_
#define SYSINIT_H_

//----------------------------------------------------------------------------//
#include "stm32f10x.h"
//#include "main.h"

//----------------------------------------------------------------------------//
#define INT_FLASH_KEY1          ((uint32_t)0x45670123)
#define INT_FLASH_KEY2          ((uint32_t)0xCDEF89AB)
#define CONFIG_MEMORY_START     ((uint32_t)0x0801F800)  // адрес, с которого будет начинаться запись во флеш (с начала 126-ой страницы F103)
#define CONFIG_MEMORY_SIZE      ((uint16_t)0X0400)      //1024 объем флеша (F103)
#define CONFIG_MEMORY_SECTOR    ((uint16_t)0X0100)      //256 размер сектора (F103)

//----------------------------------------------------------------------------//




#endif /* SYSINIT_H_ */
