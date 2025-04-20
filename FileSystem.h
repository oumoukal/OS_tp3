//
// NOM PRENOM :
// IDUL :
//
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "Inode.h"  // Pour struct Inode
#include "BlockDevice.h"        // Pour BlockDevice

/**
 * FileSystem : gère un mini-système de fichiers
 * sur un disque virtuel. Les éléments à implémenter :
 *  - freeBitmap : un vecteur<bool> pour savoir
 *    quels blocs sont libres ou occupés
 *  - root : un map (nom -> Inode) représentant
 *    le répertoire racine
 *  - device : référence au BlockDevice
 *
 * Méthodes à compléter :
 *  - Compact() : pour défragmenter le disque
 *  - AllocateBlocks() : trouver des blocs libres
 *  - FreeBlocks() : libérer des blocs
 *  - Create() : créer un fichier (allouer blocs + inode)
 *  - Write() : écrire dans un fichier à un offset donné
 *  - Read() : lire depuis un offset sur une longueur donnée
 *  - Delete() : supprimer un fichier (libérer ses blocs)
 *  - List() : afficher la liste des fichiers
 */
class FileSystem {
private:
    BlockDevice &device;                          // Disque virtuel
    std::vector<bool> freeBitmap;                 // true = libre, false = occupé
    std::unordered_map<std::string, Inode> root;  // Répertoire racine

public:
    // Constructeur : initialiser freeBitmap
    FileSystem(BlockDevice &dev);

    // Compact() : réorganiser les fichiers pour
    // éliminer la fragmentation externe
    void Compact();

    // Trouver nbBlocs libres (retourne un vecteur
    // d'indices de blocs alloués)
    std::vector<size_t> AllocateBlocks(size_t nbBlocs);

    // Libérer la liste de blocs passés en paramètre
    void FreeBlocks(const std::vector<size_t> &blocks);

    // Opérations sur fichiers
    bool Create(const std::string &filename, size_t sizeInBytes);
    bool Write(const std::string &filename, size_t offset, const std::string &data);
    bool Read(const std::string &filename, size_t offset, size_t length, std::string &outData);
    bool Delete(const std::string &filename);
    void List();
    void freeAllBlocks();
    const std::vector<bool>& getFreeBitmap() const;
};

#endif // FILESYSTEM_H