#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include "FAT.h"
#include "HAL.h"
 
 /*******************************************************************************
 * Prototypes
 ******************************************************************************/
 
/**Function print header of program*/
int PrintHeader();

/**Function read data form entrybuffer and return struct entry*/
struct DirectoryEntry EntryInfoLoad(uint8_t *EntryBuff);

/**Function read struct and show information of directory entry*/
int EntryInfoShow(struct DirectoryEntry EntryInfo);

/**Function find and return next cluster address on FAT*/
unsigned int NextCluster(unsigned int CurrentClusAddress, uint32_t FatStartSector, uint16_t BytePerSec);

/**Function show ROOT DIRECTORY TABLLE*/
int ShowRootDir(uint32_t RootDirStartSector, uint32_t RootDirSector, uint16_t BytePerSec);

/**Function show Subdirectory Table*/
int ShowSubDir(unsigned int StartSector, uint16_t BytePerSec, uint32_t FatStartSector, uint32_t DataStartSector, uint32_t SecPerClus);

/**Function show data of one cluster*/
void ShowFileCluster(unsigned int CurrentClusAddress,unsigned int DataStartSector, uint16_t BytePerSec, uint8_t SecPerClus);

/**Function show program menu*/
int ShowRootChoiceMenu();

/**Function find and read the selected entry on Root Directory and return to DirectoryEntry struct*/
struct DirectoryEntry FindAndReadSelRootEntry(uint32_t RootDirStartSector,uint32_t RootDirSector,int FileNum,uint32_t BytePerSec);

/**Function find and read the selected entry on Sub Directory and return to DirectoryEntry struct*/
struct DirectoryEntry FindAndReadSelSubEntry(int FileNum,unsigned int StartSec,uint16_t BytePerSec,uint32_t FatStartSec,uint32_t DataStartSec,uint32_t SecPerClus);

/*******************************************************************************
 * Code
 ******************************************************************************/

unsigned int NextCluster(unsigned int CurrentClusAddress, uint32_t FatStartSector, uint16_t BytePerSec)
{
	unsigned int FatSecAddress    = FatStartSector + (CurrentClusAddress + (CurrentClusAddress >> 1)) / BytePerSec;
	unsigned int FatEntOffset = (CurrentClusAddress + (CurrentClusAddress >> 1)) % BytePerSec;
	unsigned int EntryValue;
	uint8_t LastByteOfSector;
	uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	HAL_ReadSec(FatSecAddress*BytePerSec, SecBuff);
	
	if(FatEntOffset==511)                                                                           /**Special case*/
	{
		LastByteOfSector = SecBuff[FatEntOffset];
		HAL_ReadSec((FatSecAddress+1)*BytePerSec, SecBuff);
		if(CurrentClusAddress & 1)
            EntryValue = (((LastByteOfSector)&0xF0) >> 4) | (SecBuff[0] << 4);
        else
            EntryValue = (LastByteOfSector) | ((SecBuff[0]&0x0F) << 8);
	}
	else                                                                                            /**Normal case*/
	{
		if(CurrentClusAddress & 1)
			EntryValue = ((SecBuff[FatEntOffset]&0xF0) >> 4) | (SecBuff[FatEntOffset+1] << 4);
		else
			EntryValue = SecBuff[FatEntOffset] | ((SecBuff[FatEntOffset+1]&0x0F)<<8) ;
	}
	return (EntryValue);
}

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
    /**Read data and save to struct*/
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
	/**Check Dot Entry*/
	if(EntryInfo.ShortName[0]=='.')
	{
        if(EntryInfo.ShortName[1]=='.')
            strcpy(EntryInfo.ShortName,"GoBack");
        else
            strcpy(EntryInfo.ShortName,"Reload");
	}
	/**Print Entry Info*/
	printf("%-9s",EntryInfo.ShortName);
	/**File info*/
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
	/**Folder info*/
	else
	{
		printf("%-4s","");
		printf("%-8s","Folder");
		printf("%-48s","");
	}
	printf("\n");
	return 0;
}

int ShowRootDir(uint32_t RootDirStartSector, uint32_t RootDirSector, uint16_t BytePerSec)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	int CountSec 	= 0;
	int EntryOffset = 0;
	int FileNum=0;
	struct DirectoryEntry EntryInfo;
	system("cls");
	PrintHeader();
	do 
	{
	    /**Read one Sector*/
		HAL_ReadSec((RootDirStartSector+CountSec)*BytePerSec, SecBuff);
		do
		{
		    /**Read Entry*/
			FAT_ReadDirEntry(EntryOffset,SecBuff,EntryBuff);
			if(*EntryBuff != 0)
			{
				if(*(EntryBuff+RD_Attributes) != 0x0F)              /**Not a long file name entry*/
				{
					printf("%-4d",FileNum);
					EntryInfo = EntryInfoLoad(EntryBuff);
					EntryInfoShow(EntryInfo);
					FileNum+=1;
				}
				/**Increase entryoffset*/
				EntryOffset+=32;
			}
		}while((*EntryBuff != 0) && (EntryOffset < BytePerSec));    /**Until meet zero entry or finished reading a sector*/
		EntryOffset=0;
		CountSec+=1;
	}while((CountSec < RootDirSector) && (*EntryBuff != 0));        /**Until meet zero entry or finished reading all RootDirectory sector*/
	free(SecBuff);
	free(EntryBuff);
	return (FileNum);
}

int ShowRootChoiceMenu()
{
    int FileNum; 
    printf("\n--------------------------\n");
    printf("EXIT: -1\n");
	printf("Enter your choice: \n");
	fflush(stdin);
	scanf("%d",&FileNum);
	printf("--------------------------\n\n");
	return FileNum;
}

struct DirectoryEntry FindAndReadSelRootEntry(uint32_t RootDirStartSector,uint32_t RootDirSector,int FileNum,uint32_t BytePerSec)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	unsigned int CountSec=0;
	int check=-1;
	unsigned int EntryOffset=0;
	struct DirectoryEntry SelEntryInfo;
	/**Find entry offset*/
	do
    {
        HAL_ReadSec((RootDirStartSector+CountSec)*BytePerSec, SecBuff); 
        while((check!=FileNum) && (EntryOffset < BytePerSec))
        {
            if(*(SecBuff+ RD_Attributes + EntryOffset)!=0x0F)              /**Not a long file name entry*/
                check+=1;
            EntryOffset+=32;
        }
        if(check!=FileNum)
        {
            EntryOffset=0;
            CountSec+=1;
        }
    }
    while((check!=FileNum) && (CountSec<RootDirSector));
    /**Read Entry Info*/
    FAT_ReadDirEntry(EntryOffset-32,SecBuff,EntryBuff);
    SelEntryInfo = EntryInfoLoad(EntryBuff);
    
    free(SecBuff);
	free(EntryBuff);
	return (SelEntryInfo);
}

struct DirectoryEntry FindAndReadSelSubEntry(int FileNum,unsigned int StartSec,uint16_t BytePerSec,uint32_t FatStartSec,uint32_t DataStartSec,uint32_t SecPerClus)
{
    uint8_t *SecBuff = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff = (uint8_t *)malloc(EntryLength);
	unsigned int CountSec=0;
	int check=-1;
	unsigned int EntryOffset=0;
	unsigned int CurrentClusAddress;
	struct DirectoryEntry SelEntryInfo;
	CurrentClusAddress=(StartSec-DataStartSec+2)/SecPerClus;
	/**Find entry offset*/
	while(CurrentClusAddress!=0xFFF)
    {
        do
        {
            HAL_ReadSec((StartSec+CountSec)*BytePerSec, SecBuff); 
            while((check!=FileNum) && (EntryOffset <= BytePerSec))
            {
                if(SecBuff[RD_Attributes + EntryOffset]!=0x0F)                          /**Not a long file name entry*/
                    check+=1;
                EntryOffset+=32;
            }
            if(check!=FileNum)
            {
                EntryOffset=0;
                CountSec+=1;
            }
        }
        while((check!=FileNum) && (CountSec<SecPerClus));
        CountSec = 0;
        CurrentClusAddress = NextCluster(CurrentClusAddress,FatStartSec,BytePerSec);   /**Find next cluster address*/ 
        StartSec = CurrentClusAddress*SecPerClus + DataStartSec -2; 
    }
    /**Read entry info*/
    FAT_ReadDirEntry(EntryOffset-32,SecBuff,EntryBuff);
    SelEntryInfo = EntryInfoLoad(EntryBuff);
    
    free(SecBuff);
	free(EntryBuff);
	return (SelEntryInfo);
}

int ShowSubDir(unsigned int StartSector, uint16_t BytePerSec, uint32_t FatStartSector, uint32_t DataStartSector, uint32_t SecPerClus)
{
    uint8_t *SecBuff    = (uint8_t *)malloc(BytePerSec);
	uint8_t *EntryBuff  = (uint8_t *)malloc(EntryLength);
	int CountSec    = 0;
	int EntryOffset = 0;
	int FileNum=0;
	unsigned int CurrentClusAddress;
	struct DirectoryEntry EntryInfo;
	CurrentClusAddress=(StartSector-DataStartSector+2*SecPerClus)/SecPerClus;
	system("cls");
	PrintHeader();
	while(CurrentClusAddress!=0xFFF)
    {
        do 
        {
            HAL_ReadSec(StartSector*BytePerSec, SecBuff);
            do
            {
                FAT_ReadDirEntry(EntryOffset,SecBuff,EntryBuff);
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
            }while((*EntryBuff != 0) && (EntryOffset < BytePerSec));
            EntryOffset=0;
            CountSec+=1;
        }while((*EntryBuff != 0)&&(CountSec<=SecPerClus));
        CountSec = 0;
        CurrentClusAddress = NextCluster(CurrentClusAddress,FatStartSector,BytePerSec);
        StartSector = CurrentClusAddress*SecPerClus + DataStartSector -2*SecPerClus;
    }
    
	free(SecBuff);
	free(EntryBuff);
	return (FileNum);
}

void ShowFileCluster(unsigned int CurrentClusAddress,unsigned int DataStartSector, uint16_t BytePerSec, uint8_t SecPerClus)
{
	uint8_t *ClusterBuff = (uint8_t *)malloc(BytePerSec*SecPerClus);
	unsigned int CurrentSectorAddress = CurrentClusAddress*SecPerClus + DataStartSector - 2*SecPerClus;
	int Count;
	/**Read cluster*/
	HAL_ReadMultiSec(CurrentSectorAddress*BytePerSec, SecPerClus, ClusterBuff);
	/**Print Cluster*/
	for(Count=0;Count<SecPerClus*BytePerSec;Count++)
	{
		printf("%c",ClusterBuff[Count]);
	}
}

int main()
{
	uint32_t RootDirStartSector;                                    /**First sector of Root directory*/
	uint32_t RootDirSector;                                         /**Number of Root Dir Sector*/
	uint32_t FatStartSector;                                        /**First sector of FAT*/
	uint32_t FatSector;                                             /**Number of FAT sector*/
	uint32_t DataStartSector;                                       /**First sector of DATA*/
	struct BiosParameterBlock BPB;                                  /**BIOS PARAMETER BLOCK*/
	struct DirectoryEntry EntryInfo;                                /**Directory Entry Info*/
	int FileNum;
	char FileName[25];
	int TotalFileFolder;
	uint32_t CurrentSecAddress;                                     /**Current Sector Address*/
	uint32_t ParentAddress;                                         /**Parent Sector Address*/
	uint32_t CurrentClusAddress;                                    /**Current Cluster Address*/
	uint8_t *BootSecBuff = (uint8_t *)malloc(BOOT_SECTOR_BYTE);
	
	/**Open file and load BIOS PARAMETER BLOCK*/
	printf("Enter file name: ");
	fflush(stdin);
	scanf("%s",FileName);
	HAL_File_init(FileName);
	HAL_ReadBootSec(BootSecBuff);
	FAT_BPBLoad(BootSecBuff,&BPB);
	free(BootSecBuff);
	
    /**Caculate address of FAT and RootDir and DATA*/
	FatStartSector = BPB.ReservedSecCount;
	FatSector = BPB.SecPerFat * BPB.NumOfFat;
	RootDirStartSector = BPB.ReservedSecCount + FatSector;
	RootDirSector = (EntryLength * BPB.NumOfRootDirEntry) / BPB.BytePerSec;
	DataStartSector = RootDirStartSector + RootDirSector;
    
    /**Show RootDir Table*/
    TotalFileFolder = ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
    CurrentSecAddress = RootDirStartSector;
    while(FileNum!=-1)
    {
        /**Choice menu*/
        do
        {
            FileNum=ShowRootChoiceMenu();
            if((FileNum>(TotalFileFolder-1)) || FileNum<-1)
                printf("Range of options : [-1;%d]",TotalFileFolder-1);
        }
        while((FileNum>(TotalFileFolder-1)) || FileNum<-1);
        
    	if(FileNum!=-1)
        {
            /**Read selected entry info*/
            if(CurrentSecAddress==RootDirStartSector)
                EntryInfo = FindAndReadSelRootEntry(RootDirStartSector,RootDirSector,FileNum,BPB.BytePerSec);
            else
                EntryInfo = FindAndReadSelSubEntry(FileNum,CurrentSecAddress,BPB.BytePerSec,FatStartSector,DataStartSector,BPB.SecPerClus);
            /**Save folder address if user choice open file*/
            if(Subdirectory(EntryInfo.Attributes)==0)
            {
                ParentAddress=CurrentSecAddress;
            }
            
            CurrentSecAddress=(EntryInfo.StartOfFile*BPB.SecPerClus+DataStartSector-2*BPB.SecPerClus);
            
            if(Subdirectory(EntryInfo.Attributes)==1)
            {
                /**Show folder*/
                if(EntryInfo.StartOfFile == 0)
                {
                    CurrentSecAddress = RootDirStartSector;
                    TotalFileFolder = ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
                }
                else
                    TotalFileFolder=ShowSubDir(CurrentSecAddress,BPB.BytePerSec,FatStartSector,DataStartSector,BPB.SecPerClus);
            }
            else
            {
                /**Show File*/
                CurrentClusAddress=(CurrentSecAddress-DataStartSector+2*BPB.SecPerClus)/BPB.SecPerClus;
                while(CurrentClusAddress != 0xFFF)
                {
                    ShowFileCluster(CurrentClusAddress,DataStartSector,BPB.BytePerSec,BPB.SecPerClus);
                    CurrentClusAddress=NextCluster(CurrentClusAddress,FatStartSector,BPB.BytePerSec);
                }
                
                /**Show parent folder*/
                printf("\nPress enter for close file");
                fflush(stdin);
                getchar();
                printf("\n-------------------------------------------------------\n");
                CurrentSecAddress = ParentAddress;
                if(CurrentSecAddress == RootDirStartSector)
                    TotalFileFolder=ShowRootDir(RootDirStartSector,RootDirSector, BPB.BytePerSec);
                else
                   TotalFileFolder=ShowSubDir(CurrentSecAddress,BPB.BytePerSec,FatStartSector,DataStartSector,BPB.SecPerClus);
            }
        }
    }
    HAL_CloseFile();
    printf("End Program!!!");
}
