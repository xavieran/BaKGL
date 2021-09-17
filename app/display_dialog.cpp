#include "bak/dialog.hpp"

#include "bak/resourceNames.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

#include <iomanip>

unsigned GetInput()
{
    std::string in;
    std::getline(std::cin, in);
    std::stringstream ss;
    std::uint32_t input;
    ss << in;
    ss >> input;
    return input;
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    //BAK::Keywords keywords{};
    BAK::DialogStore dialog{};
    
    //dialog.ShowAllDialogs();
    
    std::stringstream ss{};
    ss << std::hex << argv[1];
    std::uint32_t key;
    ss >> key;
    auto current = BAK::Target{BAK::KeyTarget{key}};
    auto dialogSnippet = dialog.GetSnippet(current);

    bool good = true;

    while (good)
    {
        std::cout << "---------------------------------------- \n";
        std::stringstream ss;
        ss << "Current: " << current;
        if (std::holds_alternative<BAK::KeyTarget>(current))
            ss << " (" << dialog.GetTarget(std::get<BAK::KeyTarget>(current)) << ")";
        ss << "\n";
        logger.Info() << ss.str();
        logger.Info() << "Snippet: " << dialogSnippet << "\n";

        unsigned i = 0;
        for (const auto& c : dialogSnippet.GetChoices())
            std::cout << c << " (" << i++ << ")\n";
        std::cout << ">>> ";
        auto choice = GetInput();
        auto next = dialogSnippet.GetChoices()[choice];
        current = next.mTarget;
        dialogSnippet = dialog.GetSnippet(current);
    }


    return 0;
}

