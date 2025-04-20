//
// NOM PRENOM :
// IDUL :
//
#include <iostream>
#include <cassert>
#include "BlockDevice.h"         // BlockDevice
#include "FileSystem.h"   // FileSystem
#include <algorithm>


int main() {
    // Instancier le disque virtuel (64 Ko) et le FileSystem
    BlockDevice disk;
    FileSystem fs(disk);

    // Créer un fichier "test1.txt" de 30000 octets
    bool okCreate1 = fs.Create("test1.txt", 30000);
    assert(okCreate1 && "Echec de la creation de test1.txt");

    // Écrire dedans (2 messages)
    bool okWrite1 = fs.Write("test1.txt", 0, "Bonjour tout le monde!");
    assert(okWrite1 && "Echec ecriture #1 dans test1.txt");

    bool okWrite2 = fs.Write("test1.txt", 25, "Suite de texte dans test1.");
    assert(okWrite2 && "Echec ecriture #2 dans test1.txt");

    // Lire 52 octets
    std::string contenu;
    bool okRead = fs.Read("test1.txt", 0, 90, contenu);
    assert(okRead && "Echec lecture dans test1.txt");

    // Vérifier que les deux messages y figurent
    assert(contenu.find("Bonjour tout le monde!") != std::string::npos && "Texte 1 non trouvé");
    assert(contenu.find("Suite de texte dans test1.") != std::string::npos && "Texte 2 non trouvé");

    // Afficher la liste
    fs.List();

    // Créer un 2e fichier
    bool okCreate2 = fs.Create("data.bin", 20000);
    assert(okCreate2 && "Echec creation data.bin");

    // Créer un 3e fichier
    bool okCreate3 = fs.Create("text3.txt", 14000);
    assert(okCreate3 && "Echec creation text3.txt");

    // Re-lister
    fs.List();

    // Supprimer data.bin pour créer un trou (fragmentation)
    bool okDelete1 = fs.Delete("data.bin");
    assert(okDelete1 && "Echec suppression data.bin");

    // Re-lister
    fs.List();

    // Créer un 4e fichier plus grand

    /*std::cout << "[DEBUG] Blocs libres actuels : " << std::count(fs.getFreeBitmap().begin(), fs.getFreeBitmap().end(), true) << "\n";

    std::cout << "Bitmap : ";
    for (bool b : fs.getFreeBitmap())
        std::cout << (b ? '1' : '0');
    std::cout << "\n";

    fs.Compact();

    std::cout << "Bitmap : ";
    for (bool b : fs.getFreeBitmap())
        std::cout << (b ? '1' : '0');
    std::cout << "\n";*/

    bool okCreate4 = fs.Create("bigfile.bin", 10000);
    assert(okCreate4 && "Echec creation bigfile.bin");

    // Supprimer text3.txt pour aggraver la fragmentation
    bool okDelete2 = fs.Delete("text3.txt");
    assert(okDelete2 && "Echec suppression text3.txt");

    // Appeler la compaction
    fs.Compact();

    // Re-lister pour voir le résultat
    fs.List();

    std::cout << "\nTous les tests se sont déroulés sans échec !\n";
    return 0;
}