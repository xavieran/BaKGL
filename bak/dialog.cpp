#include "bak/dialog.hpp"

namespace BAK {

void Keywords::Load(FileBuffer& fb)
{
    auto length = fb.GetUint16LE();
    std::cout << "Loading keywords" << "\n";
    std::cout << "Length: " << length << "\n";

    unsigned i = 0;
    while (fb.Tell() != 0x2b8)
    {
        std::cout << "I: " << i << " " << std::hex << fb.GetUint16LE()
            << std::dec << "\n";
        i++;
    }
    i = 0;
    while (fb.GetBytesLeft() != 0)
    {
        std::cout << "Str:" << i << " :: " << fb.GetString() << "\n";
        i++;
    }
}


DialogSnippet::DialogSnippet(FileBuffer& fb, std::uint8_t dialogFile)
{
    mDisplayStyle        = fb.GetUint8();
    mActor               = fb.GetUint16LE();
    mDisplayStyle2       = fb.GetUint8();
    mDisplayStyle3       = fb.GetUint8();
    std::uint8_t choices = fb.GetUint8();
    std::uint8_t actions = fb.GetUint8();
    auto length          = fb.GetUint16LE();
    
    std::uint8_t i;

    const auto GetTarget = [dialogFile](auto rawTarget) -> Target
    {
        constexpr std::uint32_t targetBit = 0x80000000;
        if (targetBit & rawTarget)
            return KeyTarget{rawTarget & (targetBit - 1)};
        else
            return OffsetTarget{dialogFile, rawTarget};
    };

    for (i = 0; i < choices; i++)
    {
        const auto state   = fb.GetUint16LE();
        const auto choice1 = fb.GetUint16LE();
        const auto choice2 = fb.GetUint16LE();
        const auto offset  = fb.GetUint32LE();
        const auto target  = GetTarget(offset);
        // FIXME: Should work out what to do with offset == 0
        if (offset != 0)
            mChoices.emplace_back(state, choice1, choice2, target);
    }

    for (i = 0; i < actions; i++)
    {
        const auto type = fb.GetUint16LE();
        const auto& rest = fb.GetArray<8>();

        mActions.emplace_back(
            rest,
            type);
    }
    
    if (length > 0)
        mText = fb.GetString(length);
    else
        mText = "";
}

std::ostream& operator<<(std::ostream& os, const DialogResult& d)
{
    switch (d)
    {
        case DialogResult::GiveItem: return os << "GiveItem";
        case DialogResult::LoseItem: return os << "LoseItem";
        case DialogResult::SetFlag: return os << "SetFlag";
        case DialogResult::GainSkill: return os << "GainSkill";
        case DialogResult::PlaySound: return os << "PlaySound";
        default: return os << "(" << static_cast<unsigned>(d) << ")";
    }
}

std::ostream& operator<<(std::ostream& os, const DialogAction& d)
{
    os << "DA { " 
        << " type: " << d.mType
        << " rst: [" << d.mRest << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DialogChoice& d)
{
    os << std::hex << d.mState << " -> " << d.mChoice1 
    << " | " << d.mChoice2 << " " << d.mTarget << std::dec;
    return os;
}

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d)
{
    os << "[ ds: " << std::hex << +d.mDisplayStyle << " act: " << +d.mActor
        << " ds2: " << +d.mDisplayStyle2 << " ds3: " << +d.mDisplayStyle3
        << " ]" << "\n";
    
    for (const auto& action : d.mActions)
        os << "++ " << action << "\n";

    for (const auto& choice : d.mChoices)
        os << ">> " << choice << "\n";

    os << "Text [ " << d.mText << " ]" << "\n";
    os << "Next [ ";
    if (d.mChoices.size() > 0)
        os << d.mChoices.back().mTarget;
    else
        os << "None";
    os << " ]" << "\n";
    os << std::dec;

    return os;
}

DialogStore::DialogStore()
:
    mDialogMap{},
    mSnippetMap{},
    mLogger{Logging::LogState::GetLogger("DialogStore")}
{
    Load();
}

void DialogStore::Load()
{
    for (std::uint8_t dialogFile = 0; dialogFile < 32; dialogFile++)
    {
        auto fname = GetDialogFile(dialogFile);
        auto fb = FileBufferFactory::CreateFileBuffer(fname);
        unsigned dialogs = fb.GetUint16LE();
        mLogger.Debug() << "Dialog " << fname << " has: " << dialogs << " dialogs" << "\n";

        for (unsigned i = 0; i < dialogs; i++)
        {
            auto key = KeyTarget{fb.GetUint32LE()};
            const auto val = OffsetTarget{dialogFile, fb.GetUint32LE()};
            const auto [it, emplaced] = mDialogMap.emplace(
                key,
                val);
            assert(emplaced);
            auto [checkF, checkV] = it->second;
            mLogger.Spam() << std::hex << "0x" << it->first 
                << " -> 0x" << checkV << std::dec << "\n";
        }

        while (fb.GetBytesLeft() > 0)
        {
            const auto offset = OffsetTarget{dialogFile, fb.Tell()};
            auto snippet = DialogSnippet{fb, dialogFile};
            mLogger.Spam() << offset << " @ " << snippet << "\n";
            const auto& [it, emplaced] = mSnippetMap.emplace(offset, snippet);
        }
    }
}

void DialogStore::ShowAllDialogs()
{
    for (const auto& dialogKey : mDialogMap)
    {
        try
        {
            ShowDialog(dialogKey.first);
        }
        catch (const std::runtime_error&)
        {
            mLogger.Error() << "Failed to walk dialog tree: " << dialogKey.first << "\n";
        }
    }
}

void DialogStore::ShowDialog(Target dialogKey)
{
    auto snippet = std::visit(*this, dialogKey);
    bool noText = true;
    mLogger.Info() << "Dialog for key: " << std::hex << dialogKey << "\n";
    mLogger.Debug() << "Text: " << GetFirstText(snippet) << "\n";
}

const DialogSnippet& DialogStore::GetSnippet(Target target) const
{
    return std::visit(*this, target);
}

bool DialogStore::HasSnippet(Target target) const
{
    try
    {
        std::visit(*this, target);
        return true;
    }
    catch (std::runtime_error& e)
    {
        return false;
    }
}

OffsetTarget DialogStore::GetTarget(KeyTarget dialogKey)
{
    auto it = mDialogMap.find(dialogKey);
    if (it == mDialogMap.end())
    {
        std::stringstream err{};
        err << "Key not found: " << std::hex << dialogKey << std::dec;
        throw std::runtime_error(err.str());
    }
    return it->second;
}

std::string_view DialogStore::GetFirstText(const DialogSnippet& snippet) const
{
    if (snippet.GetText().length() != 0)
        return snippet.GetText();
    else
    {
        if (snippet.GetChoices().size() > 0)
            return GetFirstText(
                GetSnippet(snippet.GetChoices().front().mTarget));
        else
            return "* no text *";
    }
}

const DialogSnippet& DialogStore::operator()(KeyTarget dialogKey) const 
{
    auto it = mDialogMap.find(dialogKey);
    if (it == mDialogMap.end())
    {
        std::stringstream err{};
        err << "Key not found: " << std::hex << dialogKey << std::dec;
        throw std::runtime_error(err.str());
    }
    return (*this)(it->second);
}

const DialogSnippet& DialogStore::operator()(OffsetTarget snippetKey) const
{
    auto snip = mSnippetMap.find(snippetKey);
    if (snip == mSnippetMap.end())
    {
        std::stringstream err{};
        err << "Offset not found: " << std::hex << snippetKey << std::dec;
        throw std::runtime_error(err.str());
    }
    return snip->second;
}

std::string DialogStore::GetDialogFile(std::uint8_t i)
{
    std::stringstream ss{};
    ss << "DIAL_Z" << std::setw(2) << std::setfill('0') << +i << ".DDX";
    return ss.str();
}

DialogIndex::DialogIndex()
:
    mKeys{},
    mLogger{Logging::LogState::GetLogger("DialogIndex")}
{
    Load();
}

void DialogIndex::Load()
{
    BAK::DialogStore dialogStore{};

    auto fb = FileBufferFactory::CreateFileBuffer(DIALOG_POINTERS);

    const unsigned dialogs = fb.GetUint16LE();
    mLogger.Debug() << "Loading: " << dialogs << "\n";

    for (unsigned i = 0; i < dialogs; i++)
    {
        assert(fb.GetUint16LE() == 0);
        // Meaning of the below? Initial state of dialog?
        auto x = fb.GetUint8();
        auto y = fb.GetUint8();
        auto zero = fb.GetUint8();

        // Affects the dialog selected
        auto dialogKey = KeyTarget{fb.GetUint32LE()};
        std::cout << "#" << std::dec << i << std::hex << " " 
            << +x << " " << +y << " " << " dialogKey: " 
            << dialogKey << " target: " << dialogStore.GetTarget(dialogKey)
            << "\n";

        const auto& emplaced = mKeys.emplace_back(dialogKey);
    }
}

}
