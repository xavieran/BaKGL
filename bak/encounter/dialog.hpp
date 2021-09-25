#pragma once

#include "bak/dialogTarget.hpp"

namespace BAK::Encounter {

class Dialog {
public:
    KeyTarget mDialog;
};

std::ostream& operator<<(std::ostream& os, const Dialog&);

class DialogFactory
{
public:
    static constexpr auto sFilename = "DEF_DIAL.DAT";

    DialogFactory();

    const Dialog& Get(unsigned i) const;

private:
    void Load();
    std::vector<Dialog> mDialogs;
};

}
