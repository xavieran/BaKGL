#pragma once

#include <utility>

template <typename F>
class ScopeGuard
{
public:
    ScopeGuard(F&& f)
    :
        mFunc{std::forward<F>(f)},
        mActive{true}
    {}

    ~ScopeGuard()
    {
        if (mActive) mFunc();
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = default;
    ScopeGuard& operator=(ScopeGuard&&) = default;

    void dismiss() { mActive = false; }

private:
    F mFunc;
    bool mActive;
};

template <typename F>
ScopeGuard(F&&) -> ScopeGuard<std::decay_t<F>>;
