#include "com/ostreamMux.hpp"

#include <algorithm>
#include <string>

OStreamMux::OStreamMux()
:
    mOutputs{&std::cout}
{}

std::streamsize OStreamMux::xsputn(const char_type* s, std::streamsize n)
{
    const auto str = std::string{
        s,
        static_cast<unsigned>(n)};

    for (auto* stream : mOutputs)
        (*stream) << str;

    return n;
}

OStreamMux::int_type OStreamMux::overflow(int_type c)
{
    for (auto* stream : mOutputs)
        (*stream) << static_cast<char>(c);
    return c;
}

void OStreamMux::AddStream(std::ostream* stream)
{
    mOutputs.emplace_back(stream);
}

void OStreamMux::RemoveStream(std::ostream* stream)
{
    auto it = std::find(mOutputs.begin(), mOutputs.end(), stream);
    if (it != mOutputs.end())
        mOutputs.erase(it);
}


