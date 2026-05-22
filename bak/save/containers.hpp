#pragma once

#include "bak/container.hpp"

#include <vector>

namespace BAK {

class FileBuffer;
class GenericContainer;

Inventory LoadInventory(
    FileBuffer& fb,
    unsigned itemCount,
    unsigned capacity);

std::vector<GenericContainer> LoadShops(FileBuffer&);
std::vector<GenericContainer> LoadContainers(FileBuffer&, unsigned zone);
std::vector<GenericContainer> LoadCombatInventories(FileBuffer&);

void Save(const Inventory&, FileBuffer&);
void Save(const GenericContainer&, FileBuffer&);

}
