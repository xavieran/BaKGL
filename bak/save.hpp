#pragma once

#include "bak/character.hpp"
#include "bak/container.hpp"
#include "bak/inventory.hpp"
#include "bak/party.hpp"

#include "bak/fileBuffer.hpp"

namespace BAK {

void Save(const Inventory&, FileBuffer&);
void Save(const GenericContainer&, FileBuffer&);
void Save(const Character&, FileBuffer&);
void Save(const Party&, FileBuffer&);

}
