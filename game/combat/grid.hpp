#pragma once

#include <glm/glm.hpp>


namespace Game {

template <typename T>
class Grid
{
public:
    const T& Get(unsigned x, unsigned y) const
    {
        return mStorage[x][y];
    }

    T& Get(unsigned x, unsigned y)
    {
        return mStorage[x][y];
    }

private:
    glm::mat<8, 13, T> mStorage;
};

}
