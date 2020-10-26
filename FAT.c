#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include "FAT.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
 /**Macro read byte from buffer*/
#define Read_Byte2(Buffer,Address) (Buffer[Address]) |  ((Buffer[Address + 1]) << 8)
#define Read_Byte4(Buffer,Address) (Buffer[Address]) |  ((Buffer[Address + 1]) << 8) | ((Buffer[Address + 2]) << 16) | ((Buffer[Address + 3]) << 24)
/**Macro calculate Date*/
#define Year(Num)  (((Num) >> 9) + 1980)
#define	Month(Num) (((Num) & 0x01E0) >> 5)
#define Day(Num)   ((Num) & 0x001F)
/**Macro calculate Time*/
#define Hour(Num)  ((Num) >> 11)
#define	Min(Num)   (((Num) & 0x07E0) >>5)
#define Sec(Num)   ((Num) & 0x001F)*2
/**Macro file/folder Attributes */
#define Read_Only(Num)			((Num) & 0x01)
#define Hidden(Num)   			(((Num)& 0x02)>>1)
#define System(Num)				(((Num)& 0x04)>>2)
#define Volume_Label(Num)		(((Num)& 0x08)>>3)
#define Subdirectory(Num)		(((Num)& 0x10)>>4)
#define Archive(Num)			(((Num)& 0x20)>>5)
#define Device(Num)				(((Num)& 0x40)>>6)
#define Reserved(Num)			(((Num)& 0x80)>>7)

/**Define BIOS PARAMETER BLOCK Element Sector Address*/
#define BPB_BytePerSeC		 	0x00B
#define BPB_SecPerClus		 	0x00D
#define BPB_ReservedSecCount 	0x00E
#define BPB_NumOfFat 		 	0x010
#define BPB_NumOfRootDirEntry 	0x011
#define BPB_SecPerFat		 	0x016

/**DIRECTORY ENTRY BYTE OFFSET*/
#define RD_ShortName			0x00
#define RD_ShortExt				0x08
#define RD_Attributes			0x0B
#define RD_TimeCre				0x0E
#define RD_DateCre				0x10
#define RD_LastModTime			0x16
#define RD_LastModDate			0x18
#define RD_StartOfFile			0x1A
#define RD_SizeOfFile			0x1C

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void FAT_BPBLoad(uint8_t *BootSecBuff, struct BiosParameterBlock *BPB);
int FAT_ReadDirEntry(unsigned int EntryOffset,uint8_t *SecBuff,uint8_t *EntryBuff);
/*******************************************************************************
 * Code
 ******************************************************************************/

void FAT_BPBLoad(uint8_t *BootSecBuff, struct BiosParameterBlock *BPB)
{
	// Read BPB data form Boot Sector
	BPB->NumOfFat		   = BootSecBuff[BPB_NumOfFat];
	BPB->SecPerClus		   = BootSecBuff[BPB_SecPerClus];
	BPB->BytePerSec		   = Read_Byte2(BootSecBuff, BPB_BytePerSeC);	
	BPB->ReservedSecCount  = Read_Byte2(BootSecBuff, BPB_ReservedSecCount);
	BPB->NumOfRootDirEntry = Read_Byte2(BootSecBuff, BPB_NumOfRootDirEntry);
	BPB->SecPerFat		   = Read_Byte2(BootSecBuff, BPB_SecPerFat);
}

int FAT_ReadDirEntry(unsigned int EntryOffset,uint8_t *SecBuff,uint8_t *EntryBuff)
{
	memcpy(EntryBuff, SecBuff+EntryOffset, EntryLength);
	return 0;
}
