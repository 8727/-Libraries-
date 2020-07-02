#ifndef _W25QXX_H
#define _W25QXX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "main.h"

/* Define --------------------------------------------------------------------*/
#define W25QXX_SPI                SPI1
#define W25Qxx_CS_LOW             GPIOA->BSRR = GPIO_BSRR_BR4
#define W25Qxx_CS_HIGHT           GPIOA->BSRR = GPIO_BSRR_BS4

#define W25QXX_DUMMY_BYTE         0xA5

#define W25_WRITE_DISABLE         0x04
#define W25_WRITE_ENABLE          0x06
#define W25_CHIP_ERASE            0xC7 //0x60
#define W25_SECTOR_ERASE          0x20
#define W25_BLOCK_ERASE           0xD8
#define W25_FAST_READ             0x0B
#define W25_PAGE_PROGRAMM         0x02
#define W25_ON_RESET              0x66
#define W25_RESET                 0x99
#define W25_GET_JEDEC_ID          0x9F
#define W25_READ_STATUS_1         0x05
#define W25_READ_STATUS_2         0x35
#define W25_READ_STATUS_3         0x15
#define W25_WRITE_STATUS_1        0x01
#define W25_WRITE_STATUS_2        0x31
#define W25_WRITE_STATUS_3        0x11
#define W25_READ_UNIQUE_ID        0x4B

typedef struct{
  uint16_t  id;
  char      *name;
  uint16_t  pageSize;
  uint32_t  pageCount;
  uint32_t  sectorSize;
  uint32_t  sectorCount;
  uint32_t  blockSize;
  uint32_t  blockCount;
  uint32_t  capacityInKiloByte;
  uint8_t   statusRegister1;
  uint8_t   statusRegister2;
  uint8_t   statusRegister3;
  uint8_t   lock;
}w25qxx_t;

extern w25qxx_t w25qxx;

void W25qxxEraseChip(void);
void W25qxxEraseSector(uint32_t sectorAddr);
void W25qxxEraseBlock(uint32_t slockAddr);

uint32_t W25qxxPageToSector(uint32_t pageAddress);
uint32_t W25qxxPageToBlock(uint32_t pageAddress);
uint32_t W25qxxSectorToBlock(uint32_t sectorAddress);
uint32_t W25qxxSectorToPage(uint32_t sectorAddress);
uint32_t W25qxxBlockToPage(uint32_t blockAddress);

uint8_t W25qxxIsEmptyPage(uint32_t pageAddress, uint32_t offsetInByte);
uint8_t W25qxxIsEmptySector(uint32_t sectorAddress, uint32_t offsetInByte);
uint8_t W25qxxIsEmptyBlock(uint32_t blockAddress, uint32_t offsetInByte);

void W25qxxWriteByte(uint8_t byte, uint32_t addr);
void W25qxxWritePage(uint8_t *pBuffer, uint32_t pageAddress, uint32_t offsetInByte, uint32_t size);
void W25qxxWriteSector(uint8_t *pBuffer, uint32_t sectorAddress, uint32_t offsetInByte, uint32_t size);
void W25qxxWriteBlock(uint8_t* pBuffer, uint32_t blockAddress, uint32_t offsetInByte, uint32_t size);

void W25qxxReadByte(uint8_t *pBuffer, uint32_t bytesAddress);
void W25qxxReadBytes(uint8_t *pBuffer, uint32_t readAddr, uint32_t NumByteToRead);
void W25qxxReadPage(uint8_t *pBuffer, uint32_t pageAddress, uint32_t offsetInByte, uint32_t size);
void W25qxxReadSector(uint8_t *pBuffer, uint32_t sectorAddress, uint32_t offsetInByte, uint32_t size);
void W25qxxReadBlock(uint8_t *pBuffer, uint32_t blockAddress, uint32_t offsetInByte,uint32_t size);

uint8_t W25qxxSpi(uint8_t data);
uint8_t W25qxxInit(void);

#endif /* _W25QXX_H */
