#include "dialog.hpp"

#include "logger.hpp"

#include "FileManager.h"
#include "FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    ////unsigned dialogKey = std::atoi(argv[1]);
    //unsigned dialogKey = 0x2dc6d3; // Phillip
    //dialogKey = 0x2dc6d4; // Sumani
    //dialogKey = 0x2dc6c9;

    /*auto kfb = FileBufferFactory::CreateFileBuffer("KEYWORD.DAT");
    BAK::Keywords kw{};
    kw.Load(kfb);*/

    unsigned dialogKey = 0x186a02;//0x1e8481;//0x2dc6d3;
    logger.Info() << "Displaying dialog:" << std::hex << dialogKey << std::dec << std::endl;

    BAK::Dialog dialog{};
    dialog.LoadKeys();
    dialog.ShowDialog(dialogKey);

    return 0;
}

