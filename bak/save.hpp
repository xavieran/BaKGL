#pragma once

#include "bak/character.hpp"
#include "bak/container.hpp"
#include "bak/inventory.hpp"
#include "bak/party.hpp"

#include "bak/fileBufferFactory.hpp"

namespace BAK {

void Save(const Inventory&, FileBuffer&);
void Save(const GenericContainer&, FileBuffer&);
void Save(const Character&, FileBuffer&);
void Save(const Party&, FileBuffer&);
void Save(const WorldClock&, FileBuffer&);
void Save(const std::vector<TimeExpiringState>& storage, FileBuffer& fb);
void Save(const SpellState& spells, FileBuffer& fb);

}
