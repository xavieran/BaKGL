#include "gui/lock/lock.hpp"

#include "bak/lock.hpp"
#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

Lock::Lock(const Icons& icons, glm::vec2 pos)
:
    Widget{
        RectTag{},
        pos,
        glm::vec2{80, 120},
        glm::vec4{0},
        true
    },
    mIcons{icons},
    mShackle{
        ImageTag{},
        std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryLockIcon(0)),
        std::get<Graphics::TextureIndex>(mIcons.GetInventoryLockIcon(0)),
        glm::vec2{13, 17},
        std::get<glm::vec2>(mIcons.GetInventoryLockIcon(0)),
        true
    },
    mLock{
        ImageTag{},
        std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryLockIcon(4)),
        std::get<Graphics::TextureIndex>(mIcons.GetInventoryLockIcon(4)),
        glm::vec2{0, 54},
        std::get<glm::vec2>(mIcons.GetInventoryLockIcon(4)),
        true
    }
{
    AddChildren();
}

void Lock::AdjustShacklePosition(glm::vec2 diff)
{
    mShackle.AdjustPosition(diff);
}

void Lock::SetLocked()
{
    mShackle.SetPosition({13, 17});
}

void Lock::SetUnlocked()
{
    mShackle.SetPosition({13, 2});
}

void Lock::SetImageBasedOnLockType(BAK::LockType lock)
{
    const auto lockIndex = std::invoke([&](){
        switch (lock)
        {
        case BAK::LockType::Easy:   return 1;
        case BAK::LockType::Medium: return 2;
        case BAK::LockType::Hard:   return 3;
        case BAK::LockType::Unpickable: return 4;
        default: return 0;
        }
    });

    if (lockIndex > 1)
        mLock.SetPosition({ 6, 54});
    else
        mLock.SetPosition({ 0, 54});

    const auto [ss, ti, dims] = mIcons.GetInventoryLockIcon(lockIndex);
    mLock.SetSpriteSheet(ss);
    mLock.SetTexture(ti);
    mLock.SetDimensions(dims);
}

void Lock::AddChildren()
{
    AddChildBack(&mShackle);
    AddChildBack(&mLock);
}

}
