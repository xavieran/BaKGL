#include "bak/model.hpp"
#include "bak/dataTags.hpp"
#include "com/ostream.hpp"
#include <set>
#include <unordered_map>

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    
    std::string tbl{argv[1]};
    auto tblBuf = BAK::FileBufferFactory::Get().CreateDataBuffer(tbl);

    logger.Info() << "Loading TBL:" << tbl << std::endl;
    BAK::LoadTBL(tblBuf);
    
    return 0;
}
