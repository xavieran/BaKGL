#include "src/dialog.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d)
{
    os << "[ ds: " << std::hex << +d.mDisplayStyle << " act: " << +d.mActor
        << " ds2: " << +d.mDisplayStyle2 << " ds3: " << +d.mDisplayStyle3
        << " ]" << std::endl;
    
    for (const auto& action : d.mActions)
    {
        os << "++ " << action.mAction << std::endl;
    }

    for (const auto& choice : d.mChoices)
    {
        os << ">> " << choice.mState << " -> " << choice.mChoice1 
            << " | " << choice.mChoice2 << " " << choice.mTarget
            << std::endl;
    }

    os << "Text [ " << d.mText << " ]" << std::endl;
    os << "Next [ ";
    if (d.mChoices.size() > 0)
        os << d.mChoices.back().mTarget;
    else
        os << "None";
    os << " ]" << std::endl;
    os << std::dec;

    return os;
}

DialogStore::DialogStore()
:
    mDialogMap{},
    mSnippetMap{},
    mLogger{Logging::LogState::GetLogger("DialogStore")}
{}

void DialogStore::Load()
{
    for (std::uint8_t dialogFile = 0; dialogFile < 32; dialogFile++)
    {
        auto fname = GetDialogFile(dialogFile);
        auto fb = FileBufferFactory::CreateFileBuffer(fname);
        unsigned dialogs = fb.GetUint16LE();
        mLogger.Info() << "Dialog " << fname << " has: " << dialogs << " dialogs" << std::endl;

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
                << " -> 0x" << checkV << std::dec << std::endl;
        }

        while (fb.GetBytesLeft() > 0)
        {
            const auto offset = OffsetTarget{dialogFile, fb.Tell()};
            auto snippet = DialogSnippet{fb, dialogFile};
            mLogger.Spam() << offset << " @ " << snippet << std::endl;
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
            mLogger.Error() << "Failed to walk dialog tree: " << dialogKey.first << std::endl;
        }
    }
}

void DialogStore::ShowDialog(Target dialogKey)
{
    auto snippet = std::visit(*this, dialogKey);
    bool noText = true;
    mLogger.Info() << "Dialog for key: " << std::hex << dialogKey << std::endl;
    mLogger.Debug() << "Text: " << GetFirstText(snippet) << std::endl;
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

DialogIndex::DialogIndex(const ZoneLabel& zoneLabel)
:
    mKeys{},
    mZoneLabel{zoneLabel},
    mLogger{Logging::LogState::GetLogger("DialogIndex")}
{
}

void DialogIndex::Load()
{
    BAK::DialogStore dialogStore{};
    dialogStore.Load();

    auto fb = FileBufferFactory::CreateFileBuffer(mZoneLabel.GetDialogPointers());

    const unsigned dialogs = fb.GetUint16LE();
    mLogger.Debug() << "Loading: " << dialogs << std::endl;

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
            << std::endl;

        const auto& emplaced = mKeys.emplace_back(dialogKey);
    }
}

}
