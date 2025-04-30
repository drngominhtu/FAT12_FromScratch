#ifndef _HANDLEFILE_
#define _HANDLEFILE_

#include <stdint.h>
uint8_t readUint8(uint32_t address);
uint16_t readUint16(uint32_t address);
uint32_t readUint32(uint32_t address);
uint8_t readString(uint32_t address, char *str, uint32_t length);
uint8_t setFile(const char *fileName);
uint8_t readData(uint32_t clusterStartAddress, uint8_t* buffer, uint32_t clusterSize);
void closeFile();

#endif


