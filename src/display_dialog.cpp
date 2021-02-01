#include "dialog.hpp"

#include "logger.hpp"

#include "FileManager.h"
#include "FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    ////unsigned dialogKey = std::atoi(argv[1]);
    unsigned dialogKey = 0x2dc6d3; // Phillip
    //dialogKey = 0x2dc6d4; // Sumani
    dialogKey = 0x2dc6c9;

    logger.Info() << "Displaying dialog:" << dialogKey << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer("DIAL_Z30.DDX");
    BAK::Dialog dialog{};
    dialog.Load(fb, dialogKey);

    return 0;
}

