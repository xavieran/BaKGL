#pragma once

#include "dialog.hpp"

#include <functional>
#include <iomanip>
#include <type_traits>
#include <variant>

namespace BAK {

class DialogSources
{
public:
    static KeyTarget GetFairyChestKey(unsigned chest)
    {
        return KeyTarget{mFairyChestKey + chest};
    }

private:
    static constexpr auto mFairyChestKey = 0x19f0a0;
};

}
