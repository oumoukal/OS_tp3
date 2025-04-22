//
// Created by User on 2025-04-19.
//

#include "FileSystem.h"
#include "BlockDevice.h"

FileSystem::FileSystem(BlockDevice &dev): device(dev), freeBitmap(device.nbreBlockDisk(), true) {
    Inode inode;
    inode.blockList = {};
    inode.fileName = "/";
    inode.fileSize = 0;
    root.insert({"/", inode});
}

// Compact() : réorganiser les fichiers pour
// éliminer la fragmentation externe
void FileSystem::Compact() {
    freeAllBlocks();
    BlockDevice diskTemporaire;
    std::vector<bool> newBitmap(freeBitmap.size(), true);
    char contenu[BlockDevice::diskBlockSize()];
    size_t nextFreeBlock = 0;
    for (auto i : root) {
        std::vector<size_t> blocks = i.second.blockList;
        std::vector<size_t> newBlocks;
        for (auto j : blocks) {
            // Lire l’ancien bloc
            device.ReadBlock(j, contenu);
            // Écrire dans le prochain bloc libre du disque temporaire
            diskTemporaire.WriteBlock(nextFreeBlock, contenu);
            newBlocks.push_back(nextFreeBlock);
            newBitmap[nextFreeBlock] = false;
            ++nextFreeBlock;
        }
        // Mettre à jour le blocList dans le répertoire racine
        i.second.blockList = newBlocks;
    }

    // copier le diskTemporaire dans le disk actuel
    for (size_t y = 0; y < BlockDevice::nbreBlockDisk(); y++) {
        diskTemporaire.ReadBlock(y, contenu);
        device.WriteBlock(y, contenu);
    }

    // Mettre à jour le bitmap
    freeBitmap = newBitmap;
}

// Trouver nbBlocs libres (retourne un vecteur
// d'indices de blocs alloués)
std::vector<size_t> FileSystem::AllocateBlocks(size_t nbBlocs) {
    std::vector<size_t> blocks;
    for (size_t i = 0; i < freeBitmap.size() && blocks.size() < nbBlocs; ++i) {
        if (freeBitmap[i]) {
            blocks.push_back(i);
            freeBitmap[i] = false; // marquer le bloc comme occupé
        }
    }
    // Vérifier qu’on a bien trouvé assez de blocs
    if (blocks.size() < nbBlocs) {
        // Rendre les blocs déjà alloués
        for (size_t i : blocks) {
            freeBitmap[i] = true;
        }
        return {}; // retourne un vecteur vide si échec
    }
    return blocks;
}


bool FileSystem::Create(const std::string &filename, size_t sizeInBytes) {
    if (root.find(filename) != root.end()) return false; // fichier existe déjà

    size_t nbreBlocks = (sizeInBytes + BlockDevice::diskBlockSize() - 1) / BlockDevice::diskBlockSize();
    std::vector<size_t> blocks = AllocateBlocks(nbreBlocks);

    if (blocks.size() < nbreBlocks) return false; // pas assez de blocs libres

    Inode inode;
    inode.blockList = blocks;
    inode.fileName = filename;
    inode.fileSize = sizeInBytes;

    root[filename] = inode;
    return true;
}


bool FileSystem::Write(const std::string &filename, size_t offset, const std::string &data) {
    //determine the bloc that we are writing to
    size_t blocNumber = offset / device.diskBlockSize();
    //determine the offset within the block
    size_t blocOffset = offset % device.diskBlockSize();
    //get the block that we will be writing to
    Inode inode = root[filename];
    size_t block = inode.blockList[blocNumber];
    //get the content before the offset, anything after the offset will be re written
    char* buffer = new char[device.diskBlockSize()];
    device.ReadBlock(blocNumber, buffer); // entire bloc inserted into buffer
    std::string str(buffer);
    delete[] buffer;
    std::string newContent = str.substr(0, blocOffset) + data; //keep only the content before the offset
    //write the new content in the block 
    const char* dataToWrite = newContent.c_str();
    device.WriteBlock(blocNumber, dataToWrite);
    return true;
}

bool FileSystem::Read(const std::string &filename, size_t offset, size_t length, std::string &outData) {
    //determine the bloc that we are reading
    size_t blocNumber = offset / device.diskBlockSize();
    //determine the offset within the block
    size_t blocOffset = offset % device.diskBlockSize();
    //get the block 
    Inode inode = root[filename];
    size_t block = inode.blockList[blocNumber];
    //get the content before the offset, anything after the offset will be re written
    char* buffer = new char[device.diskBlockSize()];
    device.ReadBlock(blocNumber, buffer); // entire bloc inserted into buffer
    std::string str(buffer);
    delete[] buffer;
    outData = str.substr(blocOffset, blocOffset + length); //keep only the content after the offset and before the length 
    return true;
}

bool FileSystem::Delete(const std::string &filename) {
    Inode inode = root[filename];
    for (size_t i : inode.blockList){ //free all the bits used
        freeBitmap[i] = true;
    }
    root.erase(filename) ;//erase the file from root
    return true;
}

void FileSystem::List() {
    for (const auto& pair : root){
        std::cout << pair.first << std::endl;
    }
}

void FileSystem::freeAllBlocks() {
    for (size_t i : freeBitmap) {
        freeBitmap[i] = true;
    }
}

const std::vector<bool>& FileSystem::getFreeBitmap() const {
    return freeBitmap;
}