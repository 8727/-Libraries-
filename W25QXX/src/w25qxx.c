#include "w25qxx.h"

w25qxx_t w25qxx;

#if (FREERTOS_ON)
#define W25qxxDelay(delay)   vDelay(delay)
#include "cmsis_os.h"
#else
#define W25qxxDelay(delay)   DelayMs(delay)
#endif

uint8_t W25qxxSpi(uint8_t data){
  while(!(W25QXX_SPI->SR & SPI_SR_TXE));
  W25QXX_SPI->DR = data;
  while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
  return W25QXX_SPI->DR;
}

void W25qxxWriteEnable(void){
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_WRITE_ENABLE);
  W25Qxx_CS_HIGHT;
  W25qxxDelay(0x01);
}

void W25qxxWriteDisable(void){
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_WRITE_DISABLE);
  W25Qxx_CS_HIGHT;
  W25qxxDelay(0x01);
}

void W25qxxWaitForWriteEnd(void){
  W25qxxDelay(0x01);
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_READ_STATUS_1);
  do{
    w25qxx.statusRegister1 = W25qxxSpi(W25QXX_DUMMY_BYTE);
    W25qxxDelay(0x01);
  }
  while((w25qxx.statusRegister1 & 0x01) == 0x01);
  W25Qxx_CS_HIGHT;
}

void W25qxxEraseChip(void){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25qxxWriteEnable();

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_CHIP_ERASE);
  W25Qxx_CS_HIGHT;

  W25qxxWaitForWriteEnd();
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

void W25qxxEraseSector(uint32_t sectorAddr){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25qxxWaitForWriteEnd();
  
  sectorAddr *= w25qxx.sectorSize;
  W25qxxWriteEnable();

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_SECTOR_ERASE);

  if(w25qxx.id >= 0x19) W25qxxSpi((sectorAddr & 0xFF000000) >> 24);
  W25qxxSpi((sectorAddr & 0xFF0000) >> 16);
  W25qxxSpi((sectorAddr & 0xFF00) >> 8);
  W25qxxSpi(sectorAddr & 0xFF);
  W25Qxx_CS_HIGHT;

  W25qxxWaitForWriteEnd();
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

void W25qxxEraseBlock(uint32_t blockAddr){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25qxxWaitForWriteEnd();
  
  blockAddr = blockAddr * w25qxx.sectorSize * 16;
  W25qxxWriteEnable();

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_BLOCK_ERASE);

  if(w25qxx.id >= 0x19) W25qxxSpi((blockAddr & 0xFF000000) >> 24);
  W25qxxSpi((blockAddr & 0xFF0000) >> 16);
  W25qxxSpi((blockAddr & 0xFF00) >> 8);
  W25qxxSpi(blockAddr & 0xFF);
  W25Qxx_CS_HIGHT;

  W25qxxWaitForWriteEnd();
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

uint32_t W25qxxPageToSector(uint32_t pageAddress){
  return((pageAddress * w25qxx.pageSize) / w25qxx.sectorSize);
}

uint32_t W25qxxPageToBlock(uint32_t pageAddress){
  return((pageAddress * w25qxx.pageSize) / w25qxx.blockSize);
}

uint32_t W25qxxSectorToBlock(uint32_t sectorAddress){
  return((sectorAddress * w25qxx.sectorSize) / w25qxx.blockSize);
}

uint32_t W25qxxSectorToPage(uint32_t sectorAddress){
  return(sectorAddress * w25qxx.sectorSize) / w25qxx.pageSize;
}

uint32_t W25qxxBlockToPage(uint32_t blockAddress){
  return (blockAddress * w25qxx.blockSize) / w25qxx.pageSize;
}

uint8_t W25qxxIsEmptyPage(uint32_t pageAddress, uint32_t offsetInByte){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;

  uint8_t  pBuffer[0x0100] = {0x00,};
  uint32_t workAddress = 0x00;
  uint16_t size = 0x00;

  size = w25qxx.pageSize - offsetInByte;
  workAddress = (offsetInByte + pageAddress * w25qxx.pageSize);

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_FAST_READ);
  if(w25qxx.id >= 0x19) W25qxxSpi((workAddress & 0xFF000000) >> 24);
  W25qxxSpi((workAddress & 0xFF0000) >> 16);
  W25qxxSpi((workAddress & 0xFF00) >> 8);
  W25qxxSpi(workAddress & 0xFF);

  W25qxxSpi(0x00);

  uint16_t i = 0;
  while(size){
    while(!(W25QXX_SPI->SR & SPI_SR_TXE));
    W25QXX_SPI->DR = 0x00;
    while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
    pBuffer[i++] = W25QXX_SPI->DR;
    size--;
  }
  W25Qxx_CS_HIGHT;
  for(uint16_t i = 0x00; i < size; i++){
    if(pBuffer[i] != 0xFF){
      w25qxx.lock = false;
      return false;
    }
  }
  w25qxx.lock = false;
  return true;
}

uint8_t W25qxxIsEmptySector(uint32_t sectorAddress, uint32_t offsetInByte){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;

  uint8_t  pBuffer[0x0100] = {0x00,};
  uint32_t workAddress = 0x00;
  uint16_t s_buf = 0x0100;
  uint16_t size = 0x00;

  size = w25qxx.sectorSize - offsetInByte;
  workAddress = (offsetInByte + sectorAddress * w25qxx.sectorSize);

  uint16_t cikl = size / 0x0100;
  uint16_t cikl2 = size % 0x0100;
  uint16_t count_cikle = 0x00;

  if(size <= 0x0100){
    count_cikle = 0x01;
  }else if(cikl2 == 0x00)
      {
        count_cikle = cikl;
      }else{
        count_cikle = cikl + 0x01;
      }
      
  for(uint16_t i = 0x00; i < count_cikle; i++){
    W25Qxx_CS_LOW;
    W25qxxSpi(W25_FAST_READ);

    if(w25qxx.id >= 0x19) W25qxxSpi((workAddress & 0xFF000000) >> 24);
    W25qxxSpi((workAddress & 0xFF0000) >> 16);
    W25qxxSpi((workAddress & 0xFF00) >> 8);
    W25qxxSpi(workAddress & 0xFF);

    W25qxxSpi(0x00);

    if(size < 0x0100) s_buf = size;

    uint16_t i = 0;
    while(s_buf){
      while(!(W25QXX_SPI->SR & SPI_SR_TXE));
      W25QXX_SPI->DR = 0x00;
      while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
      pBuffer[i++] = W25QXX_SPI->DR;
      s_buf--;
    }
    
    W25Qxx_CS_HIGHT;
    for(uint16_t i = 0; i < s_buf; i++){
      if(pBuffer[i] != 0xFF){
        w25qxx.lock = false;
        return false;
      }
    }
    size = size - 0x0100;
    workAddress = workAddress + 0x0100;
  }
  w25qxx.lock = false;
  return true;
}

uint8_t W25qxxIsEmptyBlock(uint32_t blockAddress, uint32_t offsetInByte){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;

  uint8_t  pBuffer[0x0100] = {0x00,};
  uint32_t workAddress = 0x00;
  uint16_t s_buf = 0x0100;
  uint32_t size = 0x00;

  size = w25qxx.blockSize - offsetInByte;
  workAddress = (offsetInByte + blockAddress * w25qxx.blockSize);

  uint16_t cikl = size / 0x0100;
  uint16_t cikl2 = size % 0x0100;
  uint16_t count_cikle = 0x00;

  if(size <= 0x0100){
    count_cikle = 1;
  }else if(cikl2 == 0)
    {
      count_cikle = cikl;
    }else{
    count_cikle = cikl + 1;
    }

  for(uint16_t i = 0x00; i < count_cikle; i++){
    W25Qxx_CS_LOW;
    W25qxxSpi(W25_FAST_READ);

    if(w25qxx.id >= 0x19) W25qxxSpi((workAddress & 0xFF000000) >> 24);
    W25qxxSpi((workAddress & 0xFF0000) >> 16);
    W25qxxSpi((workAddress & 0xFF00) >> 8);
    W25qxxSpi(workAddress & 0xFF);

    W25qxxSpi(0x00);

    if(size < 0x0100) s_buf = size;

    uint16_t i = 0;
    while(s_buf){
    while(!(W25QXX_SPI->SR & SPI_SR_TXE));
    W25QXX_SPI->DR = 0x00;
    while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
    pBuffer[i++] = W25QXX_SPI->DR;
    s_buf--;
  }

    W25Qxx_CS_HIGHT;

    for(uint16_t i = 0x00; i < s_buf; i++){
      if(pBuffer[i] != 0xFF){
        w25qxx.lock = false;
        return false;
      }
    }
    size = size - 0x0100;
    workAddress = workAddress + 0x0100;
  }
  w25qxx.lock = false;
  return true;
}

void W25qxxWriteByte(uint8_t byte, uint32_t addr){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25qxxWaitForWriteEnd();
  W25qxxWriteEnable();
  
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_PAGE_PROGRAMM);

  if(w25qxx.id >= 0x19) W25qxxSpi((addr & 0xFF000000) >> 24);
  W25qxxSpi((addr & 0xFF0000) >> 16);
  W25qxxSpi((addr & 0xFF00) >> 8);
  W25qxxSpi(addr & 0xFF);

  W25qxxSpi(byte);

  W25Qxx_CS_HIGHT;
  W25qxxWaitForWriteEnd();
  w25qxx.lock = false;
}

void W25qxxWritePage(uint8_t *pBuffer, uint32_t pageAddress, uint32_t offsetInByte, uint32_t size){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;

  if(((size + offsetInByte) > w25qxx.pageSize) || (size == 0x00)) size = w25qxx.pageSize - offsetInByte;

  W25qxxWaitForWriteEnd();
  W25qxxWriteEnable();

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_PAGE_PROGRAMM);

  pageAddress = (pageAddress * w25qxx.pageSize) + offsetInByte;
  if(w25qxx.id >= 0x19) W25qxxSpi((pageAddress & 0xFF000000) >> 24);
  W25qxxSpi((pageAddress & 0xFF0000) >> 16);
  W25qxxSpi((pageAddress & 0xFF00) >> 8);
  W25qxxSpi(pageAddress & 0xFF);

  uint16_t i = 0x00;
  while(size){
    while(!(W25QXX_SPI->SR & SPI_SR_TXE));
    W25QXX_SPI->DR = pBuffer[i++];
    while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
    W25QXX_SPI->DR;
    size--;
  }

  W25Qxx_CS_HIGHT;
  W25qxxWaitForWriteEnd();
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

void W25qxxWriteSector(uint8_t *pBuffer, uint32_t sectorAddress, uint32_t offsetInByte, uint32_t size){
  if((size > w25qxx.sectorSize) || (size == 0x00)) size = w25qxx.sectorSize;

  uint32_t startPage;
  uint16_t bytesToWrite;
  uint32_t localOffset;

  if((offsetInByte + size) > w25qxx.sectorSize)
    bytesToWrite = w25qxx.sectorSize - offsetInByte;
  else
    bytesToWrite = size;

  startPage = W25qxxSectorToPage(sectorAddress) + (offsetInByte / w25qxx.pageSize);
  localOffset = offsetInByte % w25qxx.pageSize;

  do{
    W25qxxWritePage(pBuffer, startPage, localOffset, bytesToWrite);
    startPage++;
    bytesToWrite -= w25qxx.pageSize - localOffset;
    pBuffer += w25qxx.pageSize - localOffset;
    localOffset = 0;
  }while(bytesToWrite > 0x00);
}

void W25qxxWriteBlock(uint8_t* pBuffer, uint32_t blockAddress, uint32_t offsetInByte, uint32_t size){
  if((size > w25qxx.blockSize) || (size == 0x00))
    size = w25qxx.blockSize;

  uint32_t startPage;
  uint16_t bytesToWrite;
  uint32_t localOffset;

  if((offsetInByte + size) > w25qxx.blockSize)
    bytesToWrite = w25qxx.blockSize - offsetInByte;
  else
    bytesToWrite = size;

  startPage = W25qxxBlockToPage(blockAddress) + (offsetInByte / w25qxx.pageSize);
  localOffset = offsetInByte % w25qxx.pageSize;

  do{
    W25qxxWritePage(pBuffer, startPage, localOffset, bytesToWrite);
    startPage++;
    bytesToWrite -= w25qxx.pageSize - localOffset;
    pBuffer += w25qxx.pageSize - localOffset;
    localOffset = 0x00;
  }while(bytesToWrite > 0x00);
}

void W25qxxReadByte(uint8_t *pBuffer, uint32_t bytesAddress){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_FAST_READ);

  if(w25qxx.id >= 0x19) W25qxxSpi((bytesAddress & 0xFF000000) >> 24);
  W25qxxSpi((bytesAddress & 0xFF0000) >> 16);
  W25qxxSpi((bytesAddress& 0xFF00) >> 8);
  W25qxxSpi(bytesAddress & 0xFF);
  W25qxxSpi(0x00);

  *pBuffer = W25qxxSpi(W25QXX_DUMMY_BYTE);

  W25Qxx_CS_HIGHT;
  w25qxx.lock = false;
}

void W25qxxReadBytes(uint8_t* pBuffer, uint32_t readAddr, uint32_t size){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_FAST_READ);

  if(w25qxx.id >= 0x19) W25qxxSpi((readAddr & 0xFF000000) >> 24);
  W25qxxSpi((readAddr & 0xFF0000) >> 16);
  W25qxxSpi((readAddr& 0xFF00) >> 8);
  W25qxxSpi(readAddr & 0xFF);
  W25qxxSpi(0x00);

  uint16_t i = 0x00;
  while(size){
    while(!(W25QXX_SPI->SR & SPI_SR_TXE));
    W25QXX_SPI->DR = 0x00;
    while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
    pBuffer[i++] = W25QXX_SPI->DR;
    size--;
  }
  W25Qxx_CS_HIGHT;
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

void W25qxxReadPage(uint8_t *pBuffer, uint32_t pageAddress, uint32_t offsetInByte, uint32_t size){
  while(w25qxx.lock) W25qxxDelay(0x01);
  w25qxx.lock = true;

  if((size > w25qxx.pageSize) || (size==0)) size = w25qxx.pageSize;
  if((offsetInByte + size) > w25qxx.pageSize) size = w25qxx.pageSize - offsetInByte;

  pageAddress = pageAddress * w25qxx.pageSize + offsetInByte;
  W25Qxx_CS_LOW;

  W25qxxSpi(W25_FAST_READ);

  if(w25qxx.id >= 0x19) W25qxxSpi((pageAddress & 0xFF000000) >> 24);
  W25qxxSpi((pageAddress & 0xFF0000) >> 16);
  W25qxxSpi((pageAddress& 0xFF00) >> 8);
  W25qxxSpi(pageAddress & 0xFF);

  W25qxxSpi(0x00);

  uint16_t i = 0;
  while(size){
    while(!(W25QXX_SPI->SR & SPI_SR_TXE));
    W25QXX_SPI->DR = 0x00;
    while(!(W25QXX_SPI->SR & SPI_SR_RXNE));
    pBuffer[i++] = W25QXX_SPI->DR;
    size--;
  }
  W25Qxx_CS_HIGHT;
  W25qxxDelay(0x01);
  w25qxx.lock = false;
}

void W25qxxReadSector(uint8_t *pBuffer,uint32_t sectorAddress,uint32_t offsetInByte,uint32_t size){
  if((size > w25qxx.sectorSize) || (size==0)) size=w25qxx.sectorSize;

  uint32_t startPage;
  uint32_t bytesToRead;
  uint32_t localOffset;

  if((offsetInByte + size) > w25qxx.sectorSize)
    bytesToRead = w25qxx.sectorSize - offsetInByte;
  else
    bytesToRead = size;

  startPage = W25qxxSectorToPage(sectorAddress) + (offsetInByte / w25qxx.pageSize);
  localOffset = offsetInByte % w25qxx.pageSize;

  do{
    W25qxxReadPage(pBuffer, startPage, localOffset, bytesToRead);
    startPage++;
    bytesToRead -= w25qxx.pageSize - localOffset;
    pBuffer += w25qxx.pageSize - localOffset;
    localOffset = 0x00;
  }while(bytesToRead > 0x00);
}

void W25qxxReadBlock(uint8_t *pBuffer, uint32_t blockAddress, uint32_t offsetInByte, uint32_t size){
  if((size > w25qxx.blockSize) || (size == 0)) size = w25qxx.blockSize;

  uint32_t startPage;
  uint32_t bytesToRead;
  uint32_t localOffset;

  if((offsetInByte + size) > w25qxx.blockSize)
    bytesToRead = w25qxx.blockSize - offsetInByte;
  else
    bytesToRead = size;

  startPage = W25qxxBlockToPage(blockAddress) + (offsetInByte / w25qxx.pageSize);
  localOffset = offsetInByte % w25qxx.pageSize;	

  do{
    W25qxxReadPage(pBuffer, startPage, localOffset, bytesToRead);
    startPage++;
    bytesToRead -= w25qxx.pageSize - localOffset;
    pBuffer += w25qxx.pageSize - localOffset;
    localOffset = 0x00;
  }while(bytesToRead > 0x00);
}

uint8_t W25qxxInit(void){
  w25qxx.lock = true;
  while(GetTick() < 100) W25qxxDelay(0x01);

  W25Qxx_CS_HIGHT;
  
  GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | GPIO_CRL_CNF7);
  GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF7_1;
  GPIOA->CRL |= GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE7;
  
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  W25QXX_SPI->CR1 &= ~SPI_CR1_BR;
  W25QXX_SPI->CR1 &= ~SPI_CR1_CPOL;
  W25QXX_SPI->CR1 &= ~SPI_CR1_CPHA;
  W25QXX_SPI->CR1 &= ~SPI_CR1_DFF;
  W25QXX_SPI->CR1 &= ~SPI_CR1_LSBFIRST;
  W25QXX_SPI->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
  W25QXX_SPI->CR1 |= SPI_CR1_MSTR;
  W25QXX_SPI->CR1 |= SPI_CR1_SPE;
  
  w25qxx.id = 0x00;
  
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_ON_RESET);
  W25Qxx_CS_HIGHT;
 
  W25Qxx_CS_LOW;
  W25qxxSpi(W25_RESET);
  W25Qxx_CS_HIGHT;

  W25Qxx_CS_LOW;
  W25qxxSpi(W25_GET_JEDEC_ID);
  W25qxxSpi(0x00);
  W25qxxSpi(0x00);
  w25qxx.id = W25qxxSpi(0x00);
  W25Qxx_CS_HIGHT;
  
  switch (w25qxx.id){
    case 0x1A:  //  w25q512
      w25qxx.name = "W25Q512";
      w25qxx.blockCount = 1024;
    break;
    case 0x19:  //  w25q256
      w25qxx.name = "W25Q256";
      w25qxx.blockCount = 512;
    break;
    case 0x18:  //  w25q128
      w25qxx.name = "W25Q128";
      w25qxx.blockCount = 256;
    break;
    case 0x17:  //  w25q64
      w25qxx.name = "W25Q64";
      w25qxx.blockCount = 128;
    break;
    case 0x16:  //  w25q32
      w25qxx.name = "W25Q32";
      w25qxx.blockCount = 64;
    break;
    case 0x15:  //  w25q16
      w25qxx.name = "W25Q16";
      w25qxx.blockCount = 32;
    break;
    case 0x14:  //  w25q80
      w25qxx.name = "W25Q80";
      w25qxx.blockCount = 16;
    break;
    case 0x13:  //  w25q40
      w25qxx.name = "W25Q40";
      w25qxx.blockCount = 8;
    break;
    case 0x12:  //  w25q20
      w25qxx.name = "W25Q20";
      w25qxx.blockCount = 4;
    break;
    case 0x11:  //  w25q10
      w25qxx.name = "W25Q10";
      w25qxx.blockCount = 2;
    break;
    default:
      w25qxx.name = "XXXXXXX";
      w25qxx.blockCount = 0;
      w25qxx.lock = false;
    return false;
  }
  
  w25qxx.pageSize = 0x0100;
  w25qxx.sectorSize = 0x1000;
  w25qxx.sectorCount = w25qxx.blockCount * 0x10;
  w25qxx.pageCount = (w25qxx.sectorCount * w25qxx.sectorSize) / w25qxx.pageSize;
  w25qxx.blockSize = w25qxx.sectorSize * 0x10;
  w25qxx.capacityInKiloByte = (w25qxx.sectorCount * w25qxx.sectorSize) / 1024;
  
  w25qxx.lock = false;
  return true;
}
