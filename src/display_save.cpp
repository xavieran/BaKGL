#include "gameData.hpp"

#include "logger.hpp"

#include "worldFactory.hpp"

#include "FileManager.h"
#include "FileBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptor/indexed.hpp>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("WorldTileStore");
    
    std::string saveFile{argv[1]};

    logger.Info() << "Loading save:" << saveFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(saveFile);
    BAK::GameData gameData(fb);

    return 0;
}

