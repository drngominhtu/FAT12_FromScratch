#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "handlefile.h"
#include "readfat.h"
////////////////////////////////////////////STRUCT////////////////////////////////////////////
typedef struct{
    uint8_t jumpInstruction[3];     // Jump instruction
    char oemID[8];                  // OEM ID
    uint16_t bytesPerSector;         // Bytes per sector
    uint8_t sectorsPerCluster;       // Sectors per cluster
    uint16_t reservedSectorCount;    // Reserved sectors count
    uint8_t numberOfFATS;            // Number of FATs
    uint16_t maxRootDirEntries;      // Max root directory entries
    uint16_t totalSectors16;         // Total sectors
    uint8_t mediaDescriptor;         // Media descriptor
    uint16_t sectorsPerFAT;          // Sectors per FAT
    uint16_t sectorsPerTrack;        // Sectors per track
    uint16_t numberOfHeads;          // Number of heads
    uint32_t hiddenSectors;          // Hidden sectors
    uint32_t totalSectors32;         // Total sectors
}BootSector_t;

typedef struct{
    char name[9];          // Tên file (8 ký t?)
    char extension[4];     // Ph?n m? r?ng (3 ký t?)
    uint8_t attributes;    // Thu?c tính
    uint16_t reserved;     // Dành riêng
    uint16_t creationTime; // Th?i gian t?o
    uint16_t creationDate; // Ngày t?o
    uint16_t lastAccessDate; // Ngày truy c?p
    uint16_t highCluster;  // Cluster cao
    uint16_t lastModifiedTime; // Th?i gian s?a d?i
    uint16_t lastModifiedDate; // Ngày s?a d?i
    uint16_t startingCluster; // Cluster b?t d?u
    uint32_t fileSize;     // Kích thu?c file
    uint8_t creationTimeHour; // Th?i gian t?o
    uint16_t creationTimeMinute; // Th?i gian t?o
    uint16_t creationTimeSecond; // Th?i gian t?o
    uint16_t creationDateYear; // Ngày t?o
    uint16_t creationDateMonth; // Ngày t?o
    uint16_t creationDateDay; // Ngày t?o
}DirectoryEntry_t;
////////////////////////////////////////////STRUCT////////////////////////////////////////////

////////////////////////////////////////////VARIABLE////////////////////////////////////////////
BootSector_t bootSector; 					// const
uint32_t fatStart;							// const
uint32_t rootDirStart;						// const
uint32_t dataStart;							// const
uint32_t bytePerCluster;					// const
uint32_t entryDirectoryPerCluster; 			// const 
uint32_t g_currentCluster 	= 	0x000;		// global variable
uint32_t g_previousCluster 	=	0x000; 		// global variable
uint32_t g_numberOfEntries 	=	0x000; 		// global variable
uint8_t g_currentAttribute =	0x10; 		// global variable
////////////////////////////////////////////VARIABLE////////////////////////////////////////////

////////////////////////////////////////////FUNCTION////////////////////////////////////////////
uint8_t ReadFAT12Program(char * fileName);// MAIN 
uint8_t readBootAndSetup();// Read boot sector and set setup
uint32_t getFAT12Entry(uint32_t cluster_index);// Get FAT12 Entry
uint8_t printData(uint8_t* buffer); // print data 
uint8_t printClusterData(uint32_t startCluster);// Print data in one cluster 
DirectoryEntry_t readOneDirectoryEntry(uint32_t clusterAddress, uint32_t entryIndex);// Read directory entry
uint32_t printDirectory(uint32_t startCluster);// print entry in data directory
uint32_t printRoot();// print entry in root directory
uint32_t getClusterInRoot(uint32_t index);// get cluster index in ROOT directory
uint32_t getClusterInDirectory(uint32_t startCluster, uint32_t index);// get cluster index in directory
uint8_t getAttributeInRoot(uint32_t index);// get attribute in ROOT directory by index
uint8_t getAttributeInDirectory(uint32_t startCluster, uint32_t index);// get attribute in directory by index
uint8_t userInterface();// UI function
////////////////////////////////////////////FUNCTION////////////////////////////////////////////

// Main
uint8_t ReadFAT12Program(char * fileName){
	if (!setFile(fileName)){
        printf("Can't open file!!!\n");
        return 1;
    }
    readBootAndSetup();
	userInterface();
	closeFile();
}
// Read boot sector and set setup
uint8_t readBootAndSetup(){
    uint8_t result = 1;
    bootSector.jumpInstruction[0] = readUint8(0x00);
    bootSector.jumpInstruction[1] = readUint8(0x01);
    bootSector.jumpInstruction[2] = readUint8(0x02);
    readString(0x03, bootSector.oemID, 8);
    bootSector.bytesPerSector = readUint16(0x0B);
    bootSector.sectorsPerCluster = readUint8(0x0D);
    bootSector.reservedSectorCount = readUint16(0x0E);
    bootSector.numberOfFATS = readUint8(0x10);
    bootSector.maxRootDirEntries = readUint16(0x11);
    bootSector.totalSectors16 = readUint16(0x13);
    bootSector.mediaDescriptor = readUint8(0x15);
    bootSector.sectorsPerFAT = readUint16(0x16);
    bootSector.sectorsPerTrack = readUint16(0x18);
    bootSector.numberOfHeads = readUint16(0x1A);
    bootSector.hiddenSectors = readUint32(0x1C);
    bootSector.totalSectors32 = readUint32(0x20);
    
   	fatStart = bootSector.reservedSectorCount;
    rootDirStart = fatStart + (bootSector.sectorsPerFAT * bootSector.numberOfFATS);
    dataStart = rootDirStart + ((bootSector.maxRootDirEntries * 32 + bootSector.bytesPerSector - 1) / bootSector.bytesPerSector);
    bytePerCluster = bootSector.bytesPerSector * bootSector.sectorsPerCluster;
    entryDirectoryPerCluster = bytePerCluster / 32;
    return 1;
}
// Get FAT12 Entry
uint32_t getFAT12Entry(uint32_t cluster_index) {
    uint32_t byte_offset = fatStart*bytePerCluster + (cluster_index/2*3);
    
    uint32_t FAT_entry;

    if (cluster_index % 2 == 0) {       
        uint8_t lowByte = readUint8(byte_offset);
        uint8_t highByte = readUint8(byte_offset + 1);
        FAT_entry = (highByte & 0x0F) << 8 | lowByte;
    } else {
        uint8_t lowByte = readUint8(byte_offset + 1);
        uint8_t highByte = readUint8(byte_offset + 2);
        FAT_entry = (highByte << 4) | (lowByte >> 4);
    }
    return FAT_entry;
}

uint8_t printData(uint8_t* buffer) {
	uint16_t i; 
    for (i = 0; i < 512; i++) {
        char ch = (char)buffer[i];
        printf("%c", ch);
    }
    printf("\n");
    return 1;
}
// Print data in one cluster 
uint8_t printClusterData(uint32_t startCluster) {
	uint32_t i;
    uint32_t currentCluster = startCluster;
    uint32_t dataAddress;
    uint8_t buffer[513];
    system("@cls||clear");
    if(startCluster != 0xfff){
	    do{
        dataAddress = (dataStart + currentCluster - 2) * bytePerCluster;
        readData(dataAddress, buffer, bytePerCluster);
        printData(buffer);
		//printf("%X next: %X\n",currentCluster,readFAT12Entry(currentCluster));
        currentCluster = getFAT12Entry(currentCluster);
    	}while(currentCluster >= 0x002 && currentCluster <= 0xFF8);
		printf("\n\n");
	}else{
		printf("Empty folder\n\n");
	}
    return 1;
}

// Read directory entry
DirectoryEntry_t readOneDirectoryEntry(uint32_t clusterAddress, uint32_t entryIndex) {
    uint32_t address = clusterAddress + (entryIndex * 32);
    uint8_t buffer[32];
    uint32_t j;
    DirectoryEntry_t entry;
    readString(address, (char *)buffer, 32);
    if (buffer[0] == 0x00) {
        entry.name[0] = '\0';
        return entry;
    }
    for (j = 0; j < 8; j++) entry.name[j] = buffer[j];
    entry.name[8] = '\0';
    
    for (j = 0; j < 3; j++) entry.extension[j] = buffer[8 + j];
    entry.extension[3] = '\0';
    
    entry.attributes = buffer[11];
    entry.creationTime = (buffer[13] | (buffer[14] << 8));
    entry.creationDate = (buffer[16] | (buffer[17] << 8));
    entry.lastAccessDate = (buffer[18] | (buffer[19] << 8));
    entry.highCluster = (buffer[20] | (buffer[21] << 8));
    entry.startingCluster = (buffer[26] | (buffer[27] << 8) | (entry.highCluster << 16));
    entry.fileSize = (buffer[28] | (buffer[29] << 8) | (buffer[30] << 16) | (buffer[31] << 24));
    entry.creationTimeHour = (entry.creationTime >> 11) & 0x1F;
    entry.creationTimeMinute = (entry.creationTime >> 5) & 0x3F;
    entry.creationTimeSecond = (entry.creationTime & 0x1F) * 2;
    entry.creationDateDay = entry.creationDate & 0x1F;
    entry.creationDateMonth = (entry.creationDate >> 5) & 0x0F;
    entry.creationDateYear = ((entry.creationDate >> 9) & 0x7F) + 1980;
    return entry;
}

// print entry in data directory
uint32_t printDirectory(uint32_t startCluster) {
    uint32_t i;
    uint32_t currentCluster = startCluster;
    uint32_t entryCount = 0;
    uint32_t clusterAddress;
    DirectoryEntry_t entry;
    
    system("@cls||clear");
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");
    printf("|Index|   Name   |Extension|   Attributes    |     Created Time    |  Size   |\n");
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");
    do{
        clusterAddress = (dataStart + currentCluster - 2) * bytePerCluster;
        for(i = 0; i < bytePerCluster / 32; i++) {
            entry = readOneDirectoryEntry(clusterAddress, i);
            if (entry.name[0] == '\0') {
                continue;
            }
            char attributes[20] = "Unknown";
            if (entry.attributes == 0x01) strcpy(attributes, "Read Only");
            if (entry.attributes == 0x02) strcpy(attributes, "Hidden");
            if (entry.attributes == 0x04) strcpy(attributes, "System");
            if (entry.attributes == 0x08) strcpy(attributes, "Volume Label");
            if (entry.attributes == 0x10) strcpy(attributes, "Directory");
            if (entry.attributes == 0x20) strcpy(attributes, "Archive");
            
            printf("|%-5d| %-8s | %-3s     | %-15s |  %02u/%02u/%04u %02u:%02u   |%-9u|\n",
                entryCount + 1,
                entry.name,
                entry.extension,
                attributes,
                entry.creationDateDay,
                entry.creationDateMonth,
                entry.creationDateYear,
                entry.creationTimeHour,
                entry.creationTimeMinute,
                entry.fileSize
            );
            entryCount++;
        }
        currentCluster = getFAT12Entry(currentCluster);
    }while(currentCluster <= 0xFF8);
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");
    return entryCount;
}

// print entry in root directory
uint32_t printRoot(){
    uint32_t entryCount = 0;
    DirectoryEntry_t entry;
    uint32_t numberOfRootDirectoryCluster = dataStart - rootDirStart;
    
    uint32_t clusterAddress;
	uint32_t i;
	uint32_t j;
    system("@cls||clear");
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");
    printf("|Index|   Name   |Extension|   Attributes    |     Created Time    |  Size   |\n");
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");

    for ( i = 0; i < numberOfRootDirectoryCluster; i++) {
    	clusterAddress = (rootDirStart + i) * bytePerCluster;
        for(j = 0; j < bytePerCluster / 32; j++) {
	        entry = readOneDirectoryEntry(clusterAddress, j); 
	
	        if (entry.name[0] == '\0') {
	            continue;
	        }
			if (!(entry.startingCluster >= 0x002 && entry.startingCluster <= 0xFF8)){
	            continue;
	        }
	        char attributes[20] = "Unknown";
	        if (entry.attributes == 0x01) strcpy(attributes, "Read Only");
	        if (entry.attributes == 0x02) strcpy(attributes, "Hidden");
	        if (entry.attributes == 0x04) strcpy(attributes, "System");
	        if (entry.attributes == 0x08) strcpy(attributes, "Volume Label");
	        if (entry.attributes == 0x10) strcpy(attributes, "Directory");
	        if (entry.attributes == 0x20) strcpy(attributes, "Archive");
	
	        printf("|%-5d| %-8s | %-3s     | %-15s |  %02u/%02u/%04u %02u:%02u   |%-9u|\n",
	            entryCount + 1,
	            entry.name,
	            entry.extension,
	            attributes,
	            entry.creationDateDay,
	            entry.creationDateMonth,
	            entry.creationDateYear,
	            entry.creationTimeHour,
	            entry.creationTimeMinute,
	            entry.fileSize
	        );
	        entryCount++;
	    }
    }
    printf("+-----+----------+---------+-----------------+---------------------+---------+\n");
    return entryCount;
}

// get cluster index in ROOT directory
uint32_t getClusterInRoot(uint32_t index){
	uint32_t entryCount = 0;
    DirectoryEntry_t entry;
    uint32_t numberOfRootDirectoryCluster = dataStart - rootDirStart;
    uint32_t clusterAddress;
	uint32_t i;
	uint32_t j;
	uint32_t result = 0xfff;
    for ( i = 0; i < numberOfRootDirectoryCluster; i++) {
    	clusterAddress = (rootDirStart + i) * bytePerCluster;
        for(j = 0; j < bytePerCluster / 32; j++) {
	        entry = readOneDirectoryEntry(clusterAddress, j); 
	        if (entry.name[0] == '\0') {
	            continue;
	        }
			if (!(entry.startingCluster >= 0x002 && entry.startingCluster <= 0xFF8)){
	            continue;
	        }
	        if(index == entryCount){
				result = entry.startingCluster;
			}
			entryCount++;
	    }
    }
    return result;
}

// get cluster index in directory
uint32_t getClusterInDirectory(uint32_t startCluster, uint32_t index){
	uint32_t i;
    uint32_t currentCluster = startCluster;
    uint32_t entryCount = 0;
    uint32_t clusterAddress;
    DirectoryEntry_t entry;
    uint16_t result = 0xfff;
    
    do{
        clusterAddress = (dataStart + currentCluster - 2) * bytePerCluster;
        for(i = 0; i < bytePerCluster / 32; i++) {
            entry = readOneDirectoryEntry(clusterAddress, i);
	        if (entry.name[0] == '\0') {
	            continue;
	        }
			if (!(entry.startingCluster <= 0xFF8)){
	            continue;
	        }
	        if(index == entryCount){
				result = entry.startingCluster;
			}
			entryCount++;
        }
        currentCluster = getFAT12Entry(currentCluster);
    }while(currentCluster <= 0xFF8);
    return result;
}

// get attribute in ROOT directory by index
uint8_t getAttributeInRoot(uint32_t index){
	uint32_t entryCount = 0;
    DirectoryEntry_t entry;
    uint32_t numberOfRootDirectoryCluster = dataStart - rootDirStart;
    uint32_t clusterAddress;
	uint32_t i;
	uint32_t j;
	uint32_t result;
    for ( i = 0; i < numberOfRootDirectoryCluster; i++) {
    	clusterAddress = (rootDirStart + i) * bytePerCluster;
        for(j = 0; j < bytePerCluster / 32; j++) {
	        entry = readOneDirectoryEntry(clusterAddress, j); 
	        if (entry.name[0] == '\0') {
	            continue;
	        }
			if (!(entry.startingCluster >= 0x002 && entry.startingCluster <= 0xFF8)){
	            continue;
	        }
	        if(index == entryCount){
				result = entry.attributes;
			}
			entryCount++;
	    }
    }
    return result;
}

// get attribute in directory by index
uint8_t getAttributeInDirectory(uint32_t startCluster, uint32_t index){
	uint32_t i;
    uint32_t currentCluster = startCluster;
    uint32_t entryCount = 0;
    uint32_t clusterAddress;
    DirectoryEntry_t entry;
    uint16_t result;
    
    do{
        clusterAddress = (dataStart + currentCluster - 2) * bytePerCluster;
        for(i = 0; i < bytePerCluster / 32; i++) {
            entry = readOneDirectoryEntry(clusterAddress, i);
	        if (entry.name[0] == '\0') {
	            continue;
	        }
			if (!(entry.startingCluster <= 0xFF8)){
	            continue;
	        }
	        if(index == entryCount){
				result = entry.attributes;
			}
			entryCount++;
        }
        currentCluster = getFAT12Entry(currentCluster);
    }while(currentCluster <= 0xFF8);
    return result;
}
// UI function
uint8_t userInterface(){
	int32_t userChoose;
	uint8_t checkScanf;
	uint32_t temp;
    do{
    	if(0x10 == g_currentAttribute){					// is Dir
    		if(g_currentCluster == 0x000){
				g_numberOfEntries = printRoot();
			}else{
				g_numberOfEntries = printDirectory(g_currentCluster);
			}
			printf("[0]:Exit    [Index]:Select\nEnter: ");
		}else{ 											// is File
			g_numberOfEntries = 1;
			printClusterData(g_currentCluster);
			printf("[0]:Exit    [1]:Back\nEnter: ");
		}
    	// Scanf keyboard
		checkScanf = 0;
	    do{
			scanf("%u", &userChoose);
	        if(userChoose >= 0 && userChoose <= g_numberOfEntries){
	            checkScanf = 1;
	        }else{
	            printf("Enter positive integer < %d :",g_numberOfEntries);
	            while (getchar() != '\n');
	        }
	    }while (!checkScanf);
	    // change attribute
	    if(0 != userChoose ){
			if(g_currentAttribute == 0x10){
				g_previousCluster = g_currentCluster;
				if(g_currentCluster == 0x000){
					
					g_currentAttribute = getAttributeInRoot(userChoose-1);
					g_currentCluster = getClusterInRoot(userChoose-1);
				}else{
					g_currentAttribute = getAttributeInDirectory(g_currentCluster,userChoose-1);
					g_currentCluster = getClusterInDirectory(g_currentCluster, userChoose-1);
				}
			}else{
				
				if(1 == userChoose){
					g_currentAttribute = 0x10;
					g_currentCluster = g_previousCluster;
				}
			}
		}
	}while(0 != userChoose );
}

