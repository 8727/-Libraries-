#include "sysInit.h"

//----------------------------------------------------------------------------//
static __IO uint32_t msTicks;

//----------------------------------------------------------------------------//
void SysTick_Handler(void){ msTicks++; }

//----------------------------------------------------------------------------//
void DelayMs(uint32_t ms){ uint32_t tickStart = msTicks;
  while((msTicks - tickStart) < ms){}
}

//----------------------------------------------------------------------------//
void InitCPU(void){
  uint8_t status = 0x00;
  status = SysTick_Config(SystemCoreClock / 1000);   //1ms
  
  #if defined DEBUG
    if(status) printf("<FALSE> SysTick\r\n");
    else printf("<  OK  > SysTick\r\n");
  #endif
  
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC->APB1ENR |= RCC_APB1ENR_BKPEN;
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
  
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
}

//----------------------------------------------------------------------------//
void FlashAllErase(void){
  FLASH->KEYR = INT_FLASH_KEY1;
  FLASH->KEYR = INT_FLASH_KEY2;
  FLASH->CR |= FLASH_CR_MER;
  FLASH->CR |= FLASH_CR_STRT;
  FLASH->CR |= FLASH_CR_LOCK;
}

//----------------------------------------------------------------------------//
void ConfigsFlashErase(void){
  FLASH->KEYR = INT_FLASH_KEY1;
  FLASH->KEYR = INT_FLASH_KEY2;
  while(FLASH->SR & FLASH_SR_BSY);
  if(FLASH->SR & FLASH_SR_EOP) FLASH->SR = FLASH_SR_EOP;
  FLASH->CR |= FLASH_CR_PER;
  FLASH->AR = CONFIG_MEMORY_START;
  FLASH->CR |= FLASH_CR_STRT;
  while(!(FLASH->SR & FLASH_SR_EOP));
  FLASH->SR = FLASH_SR_EOP;
  FLASH->CR &= ~FLASH_CR_PER;
  FLASH->CR |= FLASH_CR_LOCK;
}

//----------------------------------------------------------------------------//
void FlashConfigsWrite(uint32_t* buff, uint8_t sector){
  uint32_t adr = sector * CONFIG_MEMORY_SECTOR;
  if(adr => CONFIG_MEMORY_SIZE) adr = 0x00;
  adr += CONFIG_MEMORY_START;
  FLASH->KEYR = INT_FLASH_KEY1;
  FLASH->KEYR = INT_FLASH_KEY2;
  while(FLASH->SR & FLASH_SR_BSY);
  if(FLASH->SR & FLASH_SR_EOP) FLASH->SR = FLASH_SR_EOP;
  FLASH->CR |= FLASH_CR_PG;
  for(uint16_t i = 0x00; i < CONFIG_MEMORY_SECTOR; i += 0x04){
    (__IO uint16_t*)adr = ((int16_t)buff;
    (__IO uint16_t*)adr = (uint16_t)buff;
    while(!(FLASH->SR & FLASH_SR_EOP));
    FLASH->SR = FLASH_SR_EOP;
    adr += 0x04;
  }
  FLASH->CR &= ~(FLASH_CR_PG);
  FLASH->CR |= FLASH_CR_LOCK;
}

//----------------------------------------------------------------------------//
void FlashConfigsRead(uint32_t* buff, uint8_t sector){
  uint32_t adr = sector * CONFIG_MEMORY_SECTOR;
  if(adr => CONFIG_MEMORY_SIZE) adr = 0x00;
  adr += CONFIG_MEMORY_START;
  for(uint16_t i = 0x00; i < CONFIG_MEMORY_SECTOR; i += 0x04){
    buff[i] = *(uint32_t*)adr;
    adr += 0x04;
  }
}

//----------------------------------------------------------------------------//
uint32_t FlashReadData(uint32_t address){
  return (*(__IO uint32_t*) address);
}

//----------------------------------------------------------------------------//
