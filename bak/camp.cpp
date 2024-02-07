#include "bak/camp.hpp"

#include "bak/condition.hpp"
#include "bak/skills.hpp"

#include "graphics/glm.hpp"
#include "com/logger.hpp"

namespace BAK {

CampData::CampData()
:
    mClockTicks{},
    mDaytimeShadow{}
{
    const auto& logger = Logging::LogState::GetLogger("CampData");
    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer("ENCAMP.DAT");
    mHighlightSize = {fb.GetSint16LE(), fb.GetSint16LE()};
    auto mUnknown = fb.GetSint16LE();
    auto mUnknown2 = fb.GetSint16LE();
    auto tickCount = fb.GetSint16LE();
    for (unsigned i = 0; i < 24; i++)
    {
        auto x = fb.GetSint16LE();
        auto y = fb.GetSint16LE();
        logger.Debug() << "clk: " << +x << " " << +y << "\n";
        mClockTicks.emplace_back(x, y);
    }
    mClockTwelve = {fb.GetSint16LE(), fb.GetSint16LE()};
    mClockCenter = {fb.GetSint16LE(), fb.GetSint16LE()};
    for (unsigned i = 0; i < 24; i++)
    {
        auto x = fb.GetSint16LE();
        auto y = fb.GetSint16LE();
        logger.Debug() << +x << " " << +y << "\n";
        mDaytimeShadow.emplace_back(x, y);
    }
}

const std::vector<glm::vec2>& CampData::GetClockTicks() const
{
    return mClockTicks;
}

const std::vector<glm::vec2>& CampData::GetDaytimeShadow() const
{
    return mDaytimeShadow;
}

}
