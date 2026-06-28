#include "com/random.hpp"

#include <random>

Random::Random()
    : mEngine{std::random_device{}()}
{
}

Random& Random::Get()
{
    static Random instance;
    return instance;
}

unsigned Random::Generate(unsigned min, unsigned max)
{
    if (mForcedReturn)
    {
        auto result = *mForcedReturn;
        mForcedReturn.reset();
        return result;
    }
    std::uniform_int_distribution<> dist(min, max);
    return dist(mEngine);
}

void Random::SetReturn(std::optional<unsigned> value)
{
    mForcedReturn = value;
}

unsigned GetRandomNumber(unsigned min, unsigned max)
{
    return Random::Get().Generate(min, max);
}
