#include "dialog.hpp"

#include "resourceNames.hpp"

#include "logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

#include <iomanip>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    ////unsigned dialogKey = std::atoi(argv[1]);

    //auto kfb = FileBufferFactory::CreateFileBuffer("KEYWORD.DAT");
    //BAK::Keywords kw{};
    //kw.Load(kfb);

    /*auto dialogKey = BAK::KeyTarget{0x2dc6d3};
    logger.Info() << "Displaying dialog:" << std::hex << dialogKey << std::dec << std::endl;

    BAK::DialogStore dialog{};
    dialog.LoadKeys();
    
    dialog.ShowDialog(dialogKey);
    //dialog.ShowAllDialogs();*/

    auto dialogIndex = BAK::DialogIndex{BAK::ZoneLabel{"01"}};
    dialogIndex.Load();

    return 0;
}

