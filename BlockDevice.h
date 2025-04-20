//
// NOM PRENOM :
// IDUL :
//

#ifndef BLOCKDEVICE_H
#define BLOCKDEVICE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

/**
 * BlockDevice simule un disque virtuel de 64 Ko
 * découpé en blocs de 1 Ko.
 */
class BlockDevice {
private:
    static const size_t BLOCK_SIZE = 1024;      // 1 Ko
    static const size_t NUM_BLOCKS = 64;        // 64 blocs
    std::vector<char> disk;

public:
    BlockDevice();
    bool ReadBlock(size_t blockIndex, char* buffer);
    bool WriteBlock(size_t blockIndex, const char* data);
    static size_t nbreBlockDisk();
    static size_t diskBlockSize();
};
#endif //BLOCKDEVICE_H
