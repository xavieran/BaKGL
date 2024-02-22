#pragma once

#include "bak/file/fileBuffer.hpp"

#include "bak/dialogAction.hpp"

namespace BAK {
class GameState;
}

namespace BAK::State {

void SetEventDialogAction(FileBuffer&, const SetFlag& setFlag);
bool ReadConversationItemClicked(const GameState&, unsigned eventPtr);
void SetConversationItemClicked(FileBuffer&, unsigned eventPtr);
bool CheckConversationOptionInhibited(const GameState&, unsigned eventPtr);

}
