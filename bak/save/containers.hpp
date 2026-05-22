#pragma once

#include "bak/container.hpp"

#include <vector>

namespace BAK {

class FileBuffer;

std::vector<GenericContainer> LoadShops(FileBuffer&);
std::vector<GenericContainer> LoadContainers(FileBuffer&, unsigned zone);
std::vector<GenericContainer> LoadCombatInventories(FileBuffer&);

}
