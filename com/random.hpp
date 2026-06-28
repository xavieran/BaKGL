#pragma once

#include <optional>
#include <random>

class Random
{
public:
    static Random& Get();

    unsigned Generate(unsigned min, unsigned max);

    void SetReturn(std::optional<unsigned> value);

private:
    Random();

    std::mt19937 mEngine;
    std::optional<unsigned> mForcedReturn;
};

unsigned GetRandomNumber(unsigned min, unsigned max);
