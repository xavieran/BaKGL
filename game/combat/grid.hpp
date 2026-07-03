#pragma once

#include "game/combat/types.hpp"
#include "com/bits.hpp"

#include <ostream>
#include <vector>

namespace Game::Combat {

struct GridElem
{
    BAK::GamePositionAndHeading mPos;
    std::uint16_t mState;
    Combatant* mElement{nullptr};
};

class Grid
{
public:
    Grid(
        unsigned cols,
        unsigned rows)
    :
        mCols{cols},
        mRows{rows},
        mStorage(rows * cols)
    {}

    const GridElem& Get(int x, int y) const
    {
        assert(WithinBounds(GridPos{x, y}));
        return mStorage[GetIndex({x, y})];
    }

    GridElem& Get(int x, int y)
    {
        assert(WithinBounds(GridPos{x, y}));
        return mStorage[GetIndex({x, y})];
    }

    const GridElem& Get(GridPos cell) const
    {
        assert(WithinBounds(cell));
        return mStorage[GetIndex(cell)];
    }

    GridElem& Get(GridPos cell)
    {
        assert(WithinBounds(cell));
        return mStorage[GetIndex(cell)];
    }

    unsigned GetIndex(GridPos cell) const
    {
        return cell.y * mCols + cell.x;
    }

    auto GetCols() const { return mCols; }
    auto GetRows() const { return mRows; }

    bool WithinBounds(GridPos cell) const
    {
        return cell.x >= 0 && cell.x < static_cast<int>(mCols)
            && cell.y >= 0 && cell.y < static_cast<int>(mRows);
    }

    bool CanMoveTo(GridPos target) const
    {
        if (!WithinBounds(target)) return false;

        const auto& cell = Get(target);
        return CheckBitSet(cell.mState, StateFlags::Reachable);
    }

    bool CanAttack(GridPos src, GridPos target) const
    {
        if (!WithinBounds(target)) return false;

        auto& cell = Get(target);
        return CheckBitSet(cell.mState, StateFlags::Attackable);
    }

    bool IsDisabled(GridPos target) const
    {
        if (!WithinBounds(target)) return false;

        const auto& cell = Get(target);
        return CheckBitSet(cell.mState, StateFlags::Disabled);
    }

private:
    unsigned mCols;
    unsigned mRows;
    std::vector<GridElem> mStorage;
};

inline std::ostream& operator<<(std::ostream& os, const Grid& grid)
{
    os << std::hex;
    for (unsigned _y = grid.GetRows(); _y > 0; _y--)
    {
        auto y = _y - 1;
        for (unsigned x = 0; x < grid.GetCols(); x++)
        {
            os << grid.Get(x, y).mState << " ";
        }
        os << "\n";
    }
    os << std::dec;
    return os;
}

}
