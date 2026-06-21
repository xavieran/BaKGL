#pragma once

#include <vector>

namespace Game {

template <typename T, unsigned Cols = 8, unsigned Rows = 13>
class Grid
{
public:
    Grid()
        : mStorage(Cols * Rows)
    {}

    const T& Get(unsigned x, unsigned y) const
    {
        return mStorage[y * Cols + x];
    }

    T& Get(unsigned x, unsigned y)
    {
        return mStorage[y * Cols + x];
    }

    auto GetCols() const { return Cols; }
    auto GetRows() const { return Rows; }

    bool WithinBounds(unsigned x, unsigned y) const
    {
        return x < Cols && y < Rows;
    }

private:
    std::vector<T> mStorage;
};

}
