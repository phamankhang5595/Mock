#ifndef HAL_H_INCLUDED
#define HAL_H_INCLUDED
#define BOOT_SECTOR_BYTE 512

void HAL_ReadBootSec(uint8_t *BootSecBuff);
void HAL_ReadSec(unsigned int Address, uint8_t *SecBuff);
void HAL_ReadMultiSec(unsigned int FirstAddress, int NumOFSec, uint8_t *MultiSecBuff);
void HAL_closeFile();
void HAL_File_init(char *FileName);

#endif // HAL_H_INCLUDED
