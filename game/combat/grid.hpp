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

    static constexpr auto GetCols() { return Cols; }
    static constexpr auto GetRows() { return Rows; }

private:
    std::vector<T> mStorage;
};

}
