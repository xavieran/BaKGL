#pragma once

#include "bak/file/fileBuffer.hpp"

#include "bak/dialogAction.hpp"

namespace BAK::State {

void SetEventDialogAction(FileBuffer&, const SetFlag& setFlag);
bool ReadConversationItemClicked(FileBuffer&, unsigned eventPtr);
void SetConversationItemClicked(FileBuffer&, unsigned eventPtr);
bool CheckConversationOptionInhibited(FileBuffer&, unsigned eventPtr);

}
