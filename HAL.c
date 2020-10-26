#include<stdio.h>
#include<stdint.h>
#define BOOT_SECTOR_BYTE 512
#include "HAL.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOOT_SECTOR_BYTE 512

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void HAL_ReadBootSec(uint8_t *BootSecBuff);
void HAL_ReadSec(unsigned int Address, uint8_t *SecBuff);
void HAL_ReadMultiSec(unsigned int FirstAddress, int NumOFSec, uint8_t *MultiSecBuff);
void HAL_File_init(char *FileName);
void HAL_closeFile();
/*******************************************************************************
 * Variables
 ******************************************************************************/
FILE *g_Fp=NULL;
static uint16_t s_BytePerSec;
/*******************************************************************************
 * Code
 ******************************************************************************/
 
void HAL_ReadBootSec(uint8_t *BootSecBuff)
{
	rewind(g_Fp);
	fread(BootSecBuff,1,BOOT_SECTOR_BYTE,g_Fp);
}

void HAL_ReadSec(unsigned int Address, uint8_t *SecBuff)
{
	fseek(g_Fp,Address,0);
	fread(SecBuff,1,s_BytePerSec,g_Fp);
}

void HAL_ReadMultiSec(unsigned int FirstAddress, int NumOFSec, uint8_t *MultiSecBuff)
{
	unsigned int NumOfByte = NumOFSec * s_BytePerSec;
	fseek(g_Fp,FirstAddress,0);
	fread(MultiSecBuff, 1 , NumOfByte, g_Fp);
}

void HAL_File_init(char *FileName)
{
	g_Fp=fopen(FileName,"rb");
	if(g_Fp!=NULL)
	{
		fseek(g_Fp,0x00B,0);
		fread(&s_BytePerSec,1,2,g_Fp);
	}
	else
		printf("Can not open file !!!");
}

void HAL_CloseFile()
{
	fclose(g_Fp);
}

