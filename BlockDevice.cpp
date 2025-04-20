//
// Created by User on 2025-04-19.
//

#include "BlockDevice.h"

BlockDevice::BlockDevice() : disk(BLOCK_SIZE * NUM_BLOCKS, 0) {} ;

bool BlockDevice::ReadBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= NUM_BLOCKS || buffer == nullptr) return false;
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        buffer[i] = disk[blockIndex * BLOCK_SIZE + i];
    }
    return true;
};

bool BlockDevice::WriteBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= NUM_BLOCKS || data == nullptr) return false;
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        disk[blockIndex * BLOCK_SIZE + i] = data[i];
    }
    return true;
};

size_t BlockDevice::nbreBlockDisk() {
    return NUM_BLOCKS;
}

size_t BlockDevice::diskBlockSize() {
    return BLOCK_SIZE;
}

