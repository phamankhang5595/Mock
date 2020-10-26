#ifndef _HEADER_FAT_H_
#define _HEADER_FAT_H_

#define RD_ShortName_Length				8
#define RD_ShortExt_Length				3
#define EntryLength 32

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

struct BiosParameterBlock
{
	uint16_t BytePerSec;                                    // Byte Per Sector
	uint16_t ReservedSecCount;                              // Count of reserved logical sectors
	uint8_t  SecPerClus;                                    // Sector Per Cluster
	uint8_t  NumOfFat;                                      // Number of File Allocation Tables
	uint16_t NumOfRootDirEntry;                             // Number of Root Directory Entries
	uint16_t SecPerFat;                                     // Sector Per File Allocation Table
};
	
struct DirectoryEntry
{
	char ShortName[9];                             // Short file name
	char ShortExtension[4];                        // Short file Extension
	uint8_t Attributes;                                     // File attributes 
	uint16_t TimeCre;                                       // Create time	
	uint16_t DateCre;                                       // Create date
	uint16_t LastModTime;                                   // Last modified time
	uint16_t LastModDate;                                   // Last modified date
	uint16_t StartOfFile;                                   // Start of file in clusters
	uint16_t FileSz;                                        // File size in byte
};
/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Read Bios Parameter Block info
 *
 * @param *BootSecBuff  <Boot sector Buffer>.
 * @param BPB           <used to storage the information of Bios>.
 *
 * @return <BPB struct>
 */
void FAT_BPBLoad(uint8_t *BootSecBuff, struct BiosParameterBlock *BPB);

/*!
 * @brief <Read directory entry>
 *
 * @param EntryOffset <Offset of the entry>.
 * @param SecBuff     <Sector buffer>.
 * @param EntryBuff   <Entry buffer>
 * @return <Data of entry buffer>
 */
int FAT_ReadDirEntry(unsigned int EntryOffset,uint8_t *SecBuff, uint8_t *EntryBuff);
#endif /** _HEADER_FAT_H_ */
