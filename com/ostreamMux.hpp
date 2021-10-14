#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

class OStreamMux : public std::streambuf
{
public:
    OStreamMux();

    std::streamsize xsputn(
        const char_type* s,
        std::streamsize n) override;
    int_type overflow(int_type c) override;

    void AddStream(std::ostream* stream);
    void RemoveStream(std::ostream* stream);

private:
    std::vector<std::ostream*> mOutputs;
};


