#include "bak/random.hpp"

#include "com/random.hpp"

namespace BAK {

unsigned GetRandom()
{
    return GetRandomNumber(0, 0xffff);
}

}
