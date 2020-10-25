#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include "FAT.h"
#include "HAL.h"

int PrintHeader()
{
	printf("%-4s","Num");
	printf("%-9s","Name");
	printf("%-4s","Ext");
	printf("%-8s","Type");
	printf("%-13s","Cre date");
	printf("%-11s","Cre time");
	printf("%-13s","Mod date");
	printf("%-11s","Mod time");
	printf("%-8s\n","File size");
	return 0;
}

struct DirectoryEntry EntryInfoLoad(uint8_t *EntryBuff)
{
    struct DirectoryEntry EntryInfo;
    
    memcpy(EntryInfo.ShortName,EntryBuff,RD_ShortName_Length);
	EntryInfo.ShortName[RD_ShortName_Length]= '\0';
	memcpy(EntryInfo.ShortExtension,EntryBuff + RD_ShortExt,RD_ShortExt_Length);
	EntryInfo.ShortExtension[RD_ShortExt_Length]= '\0';
	EntryInfo.Attributes    = EntryBuff[RD_Attributes];
	EntryInfo.DateCre       = Read_Byte2(EntryBuff, RD_DateCre);
	EntryInfo.TimeCre       = Read_Byte2(EntryBuff, RD_TimeCre);
	EntryInfo.LastModDate   = Read_Byte2(EntryBuff, RD_LastModDate);
	EntryInfo.LastModTime   = Read_Byte2(EntryBuff, RD_LastModTime);
	EntryInfo.StartOfFile   = Read_Byte2(EntryBuff, RD_StartOfFile);
	EntryInfo.FileSz        = Read_Byte4(EntryBuff, RD_SizeOfFile);
	return EntryInfo;
}

int EntryInfoShow(struct DirectoryEntry EntryInfo)
{	
	/**< check Dot Entry */
	if(EntryInfo.ShortName[0]==0x2E)
	{
        if(EntryInfo.ShortName[1]=='.')
            strcpy(&EntryInfo.ShortName,"GoBack");
        else
            strcpy(&EntryInfo.ShortName,"Reload");
	}
	/**< Print Entry Info */
	printf("%-9s",EntryInfo.ShortName);
	
	if(Subdirectory(EntryInfo.Attributes)==0)
	{
		printf("%-4s",EntryInfo.ShortExtension);
		printf("%-8s","File");
		printf("%.2d/%.2d/%.2d   ",Day(EntryInfo.DateCre), Month(EntryInfo.DateCre), Year(EntryInfo.DateCre));
		printf("%.2d:%.2d:%.2d   ",Hour(EntryInfo.TimeCre), Min(EntryInfo.TimeCre), Sec(EntryInfo.TimeCre));
		printf("%.2d/%.2d/%.2d   ",Day(EntryInfo.LastModDate), Month(EntryInfo.LastModDate), Year(EntryInfo.LastModDate));
		printf("%.2d:%.2d:%.2d   ",Hour(EntryInfo.LastModTime), Min(EntryInfo.LastModTime), Sec(EntryInfo.LastModTime));
		printf("%d",EntryInfo.FileSz);
	}
	else
	{
		printf("%-4s","");
		printf("%-8s","Folder");
		printf("%-48s","");
	}
	
	printf("\n");
	return 0;
}

unsigned int ShowRootDir(uint32_t RootDirStartSector, uint32_t RootDirSector, uint16_t BytePerSec)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	int CountSec 	= 0;
	int EntryOffset = 0;
	int FileNum=0;
	struct DirectoryEntry EntryInfo;
	PrintHeader();
	do 
	{
		HAL_ReadSec(RootDirStartSector*BytePerSec, SecBuff);
		do
		{
			FAT_ReadRootEntry(EntryOffset,SecBuff,EntryBuff);
			if(*EntryBuff != 0)
			{
				if(*(EntryBuff+RD_Attributes) != 0x0F)
				{
					printf("%-4d",FileNum);
					EntryInfo = EntryInfoLoad(EntryBuff);
					EntryInfoShow(EntryInfo);
					FileNum+=1;
				}
				EntryOffset+=32;
			}
			
		}while((*EntryBuff != 0) && (EntryOffset <= BytePerSec));
		CountSec+=1;
	}while((CountSec < RootDirSector) &&(*EntryBuff != 0));
	free(SecBuff);
	free(EntryBuff);
	return 0;
}

int ShowRootChoiceMenu()
{
    int FileNum; 
    printf("\n--------------------------\n");
    printf("Enter -1 for EXIT\n");
	printf("Enter your choice: \n");
	fflush(stdin);
	scanf("%d",&FileNum);
	printf("--------------------------\n\n");
	return FileNum;
}

struct DirectoryEntry FindAndReadSelEntry(uint32_t StartSectorAddrees,uint32_t TotalSector,int FileNum,uint32_t BytePerSec)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	unsigned int CountSec=0;
	unsigned int check=-1;
	unsigned int EntryOffset=0;
	struct DirectoryEntry SelEntryInfo;
	/**find entry offset*/
	do
    {
        HAL_ReadSec((StartSectorAddrees+CountSec)*BytePerSec, SecBuff);
        
        while((check!=FileNum) && (EntryOffset <= BytePerSec))
        {
            if(SecBuff[RD_Attributes + EntryOffset]!=0x0F)            // Not a long file name
                check+=1;
            EntryOffset+=32;
        }
        if(check!=FileNum)
        {
            EntryOffset=0;
            CountSec+=1;
        }
    }
    while((check!=FileNum));
    FAT_ReadRootEntry(EntryOffset-32,SecBuff,EntryBuff);
    SelEntryInfo = EntryInfoLoad(EntryBuff);
    free(SecBuff);
	free(EntryBuff);
	return (SelEntryInfo);
}

unsigned int ShowSubDir(unsigned int StartSector, uint16_t BytePerSec)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	int CountSec 	= 0;
	int EntryOffset = 0;
	int FileNum=0;
	struct DirectoryEntry EntryInfo;
	PrintHeader();
	do 
	{
		HAL_ReadSec(StartSector*BytePerSec, SecBuff);
		do
		{
			FAT_ReadRootEntry(EntryOffset,SecBuff,EntryBuff);
			if(*EntryBuff != 0)
			{
				if(*(EntryBuff+RD_Attributes) != 0x0F)
				{
					printf("%-4d",FileNum);
					EntryInfo = EntryInfoLoad(EntryBuff);
					EntryInfoShow(EntryInfo);
					FileNum+=1;
				}
				EntryOffset+=32;
			}
		}while((*EntryBuff != 0) && (EntryOffset <= BytePerSec));
		CountSec+=1;
	}while(*EntryBuff != 0);
	free(SecBuff);
	free(EntryBuff);
	return 0;
}

void ShowFileCluster(unsigned int CurrentClusAddress,unsigned int DataStartSector, uint16_t BytePerSec, uint8_t SecPerClus)
{
	uint8_t *ClusterBuff = (uint8_t *)malloc(BytePerSec*SecPerClus);
	unsigned int CurrentSectorAddress = CurrentClusAddress + DataStartSector -2;
	int Count;
	HAL_ReadMultiSec(CurrentSectorAddress*BytePerSec, SecPerClus, ClusterBuff);
	/**Print Cluster*/
	for(Count=0;Count<SecPerClus*BytePerSec;Count++)
	{
		printf("%x ",ClusterBuff[Count]);
	}
}

unsigned int NextCluster(unsigned int CurrentClusAddress, unsigned int FatStartSector, uint16_t BytePerSec)
{
	unsigned int FatSecAddress    = FatStartSector + (CurrentClusAddress + (CurrentClusAddress >> 1)) / BytePerSec;
	unsigned int FatEntOffset = (CurrentClusAddress + (CurrentClusAddress >> 1)) % BytePerSec;
	unsigned int EntryValue;
	unsigned int LastByteOfSector;
	uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	HAL_ReadSec(FatSecAddress*BytePerSec, SecBuff);
	if(FatEntOffset==511)
	{
		LastByteOfSector = SecBuff[FatEntOffset];
		HAL_ReadSec((FatSecAddress+1)*BytePerSec, SecBuff);
		EntryValue = (LastByteOfSector&0xF0 >> 4) | (SecBuff[1] <<4);
	}
	else
	{
		if(CurrentClusAddress & 1)
			EntryValue = ((SecBuff[FatEntOffset]&0xF0) >> 4) | (SecBuff[FatEntOffset+1] << 4);
		else
			EntryValue = SecBuff[FatEntOffset] | ((SecBuff[FatEntOffset+1]&0x0F)<<8) ;
	}
	return (EntryValue);
}

int main()
{
	char num;
	uint32_t RootDirStartSector;							// First sector of Root directory
	uint32_t RootDirSector;									// Number of Root Dir Sector
	uint32_t FatStartSector;								// First sector of FAT
	uint32_t FatSector;										// Number of FAT sector
	uint32_t DataStartSector;								// First sector of DATA
	//uint32_t DataSector;									// Number of DATA sector
	struct BiosParameterBlock BPB;							// BIOS PARAMETER BLOCK
	struct DirectoryEntry EntryInfo;						// Directory Entry Info
	int FileNum;
	uint32_t CurrentAddress;
	uint32_t ParentAddress;
	uint32_t CurrentClusAddress;
	uint8_t *BootSecBuff = (uint8_t *)malloc(BOOT_SECTOR_BYTE);
	
	/**Open file and load BIOS PARAMETER BLOCK*/
	HAL_File_init("floppy.img");
	HAL_ReadBootSec(BootSecBuff);
	FAT_BPBLoad(BootSecBuff,&BPB);
	free(BootSecBuff);
	
    /**Caculate address of FAT and RootDir and DATA*/
	FatStartSector = BPB.ReservedSecCount;
	FatSector = BPB.SecPerFat * BPB.NumOfFat;
	RootDirStartSector = BPB.ReservedSecCount + FatSector;
	RootDirSector = (EntryLength * BPB.NumOfRootDirEntry) / BPB.BytePerSec;
	DataStartSector = RootDirStartSector + RootDirSector;
    
    /**/
    ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
    CurrentAddress = RootDirStartSector;
    while(FileNum!=-1)
    {
    	FileNum = ShowRootChoiceMenu();
    	if(FileNum!=-1)
        {
            EntryInfo = FindAndReadSelEntry(CurrentAddress,0,FileNum, BPB.BytePerSec);
            if(Subdirectory(EntryInfo.Attributes)==0)
            {
                ParentAddress=CurrentAddress;
            }
            CurrentAddress=(EntryInfo.StartOfFile*BPB.SecPerClus+DataStartSector-2);
            
            if(Subdirectory(EntryInfo.Attributes)==1)                                               /**Show folder*/
            {
                if(EntryInfo.StartOfFile == 0)
                {
                    CurrentAddress = RootDirStartSector;
                    ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
                }
                else
                    ShowSubDir(CurrentAddress,BPB.BytePerSec); // chỉnh lại đọc theo cluster
            }
            else																					/**Show File*/
            {
                CurrentClusAddress=(CurrentAddress-DataStartSector+2)/BPB.SecPerClus;
                while(CurrentClusAddress != 0xFFF)
                {
                    ShowFileCluster(CurrentClusAddress,DataStartSector,BPB.BytePerSec,BPB.SecPerClus);
                    CurrentClusAddress=NextCluster(CurrentClusAddress,FatStartSector,BPB.BytePerSec);
                }
                printf("\nPress enter for close file");
                fflush(stdin);
                getchar();
                printf("\n-------------------------------------------------------\n");
                CurrentAddress = ParentAddress;
                /**Show parent folder*/
                if(CurrentAddress == RootDirStartSector)
                    ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
                else
                    ShowSubDir(CurrentAddress,BPB.BytePerSec);
            }
        }
	}
}

