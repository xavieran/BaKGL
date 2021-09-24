#pragma once

#include "bak/dialogTarget.hpp"

namespace BAK::Encounters {


class Dialog {
public:
    KeyTarget mDialog;
};

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
