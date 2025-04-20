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
    return false;
}

bool FileSystem::Read(const std::string &filename, size_t offset, size_t length, std::string &outData) {
    return false;
}

bool FileSystem::Delete(const std::string &filename) {
    return false;
}

void FileSystem::List() {

}

void FileSystem::freeAllBlocks() {
    for (size_t i : freeBitmap) {
        freeBitmap[i] = true;
    }
}

const std::vector<bool>& FileSystem::getFreeBitmap() const {
    return freeBitmap;
}