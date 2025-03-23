#include "bak/layout.hpp"
#include "graphics/glm.hpp"

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    if (argc < 2)
    {
        std::cerr << "No arguments provided!\nUsage: "
            << argv[0] << " REQ\n";
        return -1;
    }
    
    std::string reqFile{argv[1]};

    auto layout = BAK::Layout(reqFile);

    std::stringstream ss{};
    ss << "i \t  widget \t action \t vis \t pos \t dim \t tele \t img \t grp \t label\n";
    for (unsigned i = 0; i < layout.GetSize(); i++)
    {
        const auto& widget = layout.GetWidget(i);
        ss << i << "\t" << widget.mWidget << "\t" << widget.mAction << "\t"
            << widget.mIsVisible << "\t" << widget.mPosition << "\t"
            << widget.mDimensions << "\t" << static_cast<int>(widget.mTeleport) << "\t"
            << widget.mImage << "\t" << widget.mGroup << "\t"
            << "\"" << widget.mLabel << "\"\n";
    }

    logger.Info() << ss.str() << std::endl;

    return 0;
}
