//
// NOM PRENOM :
// IDUL :
//

#ifndef INODE_H
#define INODE_H

#include <string>
#include <vector>

/**
 * Inode : décrit un fichier (nom, taille, liste de blocs).
 * Dans ce TP, un fichier est représenté par un Inode,
 * qui contient :
 *  - fileName : le nom du fichier
 *  - fileSize : la taille du fichier en octets
 *  - blockList : la liste des indices de blocs occupés
 */
struct Inode {
    std::string fileName;
    size_t fileSize;
    std::vector<size_t> blockList;
};

#endif // INODE_H