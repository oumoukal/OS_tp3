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
    std::cout << "Write dans " << filename << " ("<< data.size() <<" octets)"<<std::endl;
    //determine the bloc that we are writing to
    size_t blocNumber = offset / device.diskBlockSize();
    //determine the offset within the block
    size_t blocOffset = offset % device.diskBlockSize();
    //get the inode of the file we will be writing to
    Inode inode = root[filename];
    //get the content before the offset, anything after the offset will be rewritten
    size_t block = inode.blockList[blocNumber];
    char* buffer = new char[device.diskBlockSize()];
    device.ReadBlock(block, buffer); // entire bloc inserted into buffer
    std::string str(buffer);
    delete[] buffer;
    std::string newContent = str.substr(0, blocOffset) + data; //keep only the content before the offset append new data to the end
    //verify that we have enough space allocated to write our blocks if not returns false code be changed to write until full
    if(inode.blockList.size() < blocNumber + newContent.size() / device.diskBlockSize() + 1) return false;
    //write the new content in the block if it can fit in only one  
    if (newContent.size() <= device.diskBlockSize()){
        size_t block = inode.blockList[blocNumber];
        const char* dataToWrite = newContent.c_str();
        device.WriteBlock(block, dataToWrite);
        return true;
    }
    //if we cant fit in 1 bloc we will need to write to multiple blocs
    else{
        int numberOfBlocks = newContent.size() / device.diskBlockSize() + 1;
        int startPosition = 0;
        for (int i = 0; i< numberOfBlocks; i++ ){
            size_t block = inode.blockList[blocNumber];
            const char* dataToWrite = newContent.substr(startPosition, device.diskBlockSize()).c_str();
            device.WriteBlock(block, dataToWrite);
            startPosition += device.diskBlockSize();
            blocNumber++;
        }
    }
    return false;
    
}

bool FileSystem::Read(const std::string &filename, size_t offset, size_t length, std::string &outData) {
    std::cout << "Read dans " << filename << " ("<< length <<" octets)"<<std::endl;
    //determine the bloc that we are reading
    size_t blocNumber = offset / device.diskBlockSize();
    //determine the offset within the block
    size_t blocOffset = offset % device.diskBlockSize();
    //get the block 
    Inode inode = root[filename];
    //we need to verify we are not reading data which is not contanied in our file
    if (inode.blockList.size() < blocNumber + (length + offset) / device.diskBlockSize() + 1) return false;
    //write the new content in the block if it can fit in only one  
    if (length + offset <= device.diskBlockSize()){
        size_t block = inode.blockList[blocNumber];
        //get the block, anything after the offset will be returned
        char* buffer = new char[device.diskBlockSize()];
        device.ReadBlock(block, buffer); // entire bloc inserted into buffer
        std::string str(buffer);
        delete[] buffer;
        outData = str.substr(blocOffset, blocOffset + length); //keep only the content after the offset and before the length 
        return true;
    }
    else{
        int numberOfBlocks = (length + offset) / device.diskBlockSize() + 1;
        for (int i = 0; i< numberOfBlocks; i++ ){
            size_t block = inode.blockList[blocNumber];
            char* buffer = new char[device.diskBlockSize()];
            device.ReadBlock(block, buffer); 
            blocNumber++;
            std::string str(buffer);
            delete[] buffer;
            outData += str;
        }
        outData = outData.substr(blocOffset, blocOffset + length); //keep only the content after the offset and before the length 
    }
    return false;

}

bool FileSystem::Delete(const std::string &filename) {
    if(root.find(filename) != root.end()){
        Inode inode = root[filename];
        for (size_t i : inode.blockList){ //free all the bits used
            freeBitmap[i] = true;
        }
        root.erase(filename) ;//erase the file from root
        std::cout << "Fichier " << filename << " supprime"<<std::endl;
        return true;
    }
    else{
        return false;
    }
}

void FileSystem::List() {
    std::cout << "Liste des fichiers:"<<std::endl;
    for (const auto& pair : root){
        std::cout <<"-"<< pair.first << ": size " <<pair.second.fileSize<< " octets, nombre de blocs ="<<pair.second.blockList.size() <<std::endl;
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