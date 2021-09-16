#include "bak/party.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const Party& party)
{
    os << "Party {\n\tGold: " << party.mGold << " royals\n";
    os << "\tKeys: " << party.mKeys << "\n";
    os << "\tCharacters: \n";
    for (const auto& c : party.mCharacters)
        os << c;
    os << "\tActive: ";
    for (const auto i : party.mActiveCharacters)
        os << " " << party.mCharacters[i].mName << " ";
    os << "\n}";

    return os;
}

}
