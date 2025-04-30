#include <stdint.h>
#include <stdio.h>
#include "handlefile.h"

FILE *file;

uint8_t readUint8(uint32_t address) {
    fseek(file, address, SEEK_SET);
    uint8_t value;
    if (fread(&value, 1, 1, file) != 1) {
        return 0;
    }
    return value;
}

uint16_t readUint16(uint32_t address) {
    fseek(file, address, SEEK_SET);
    uint16_t value;
    if (fread(&value, 2, 1, file) != 1) {
        return 0;
    }
    return value;
}

uint32_t readUint32(uint32_t address) {
    fseek(file, address, SEEK_SET);
    uint32_t value;
    if (fread(&value, 4, 1, file) != 1) {
        return 0;
    }
    return value;
}

uint8_t readString(uint32_t address, char *str, uint32_t length) {
    fseek(file, address, SEEK_SET);
    fread(str, 1, length, file);
    str[length] = '\0';
    return 1;
}

uint8_t setFile(const char *fileName) {
    uint8_t result = 1;
    file = fopen(fileName, "rb");
    if (!file) {
        result = 0;
    }
    return result;
}

uint8_t readData(uint32_t clusterStartAddress, uint8_t* buffer, uint32_t clusterSize) {
    fseek(file, clusterStartAddress, SEEK_SET);
    fread(buffer, 1, clusterSize, file);
}

void closeFile() {
    fclose(file);
}

