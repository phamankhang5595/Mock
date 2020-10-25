#include<stdio.h>
#include<stdint.h>
#define BOOT_SECTOR_BYTE 512
#include "HAL.h"
FILE *fp=NULL;
static uint16_t BytePerSec;
/**Read 512 byte Boot System data*/
void HAL_ReadBootSec(uint8_t *BootSecBuff)
{
	rewind(fp);
	fread(BootSecBuff,1,BOOT_SECTOR_BYTE,fp);
}

void HAL_ReadSec(unsigned int Address, uint8_t *SecBuff)
{
	fseek(fp,Address,0);
	fread(SecBuff,1,BytePerSec,fp);
}

void HAL_ReadMultiSec(unsigned int FirstAddress, int NumOFSec, uint8_t *MultiSecBuff)
{
	unsigned int NumOfByte = NumOFSec * BytePerSec;
	fseek(fp,FirstAddress,0);
	fread(MultiSecBuff, 1 , NumOfByte, fp);
}

void HAL_File_init(char *FileName)
{
	fp=fopen(FileName,"rb");
	if(fp!=NULL)
	{
		fseek(fp,0x00B,0);
		fread(&BytePerSec,1,2,fp);
	}
	else
		printf("Can not open file !!!");
}

void HAL_closeFile()
{
	fclose(fp);
}

