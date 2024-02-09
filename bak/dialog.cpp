#include "bak/dialog.hpp"

#include "com/assert.hpp"

namespace BAK {

Keywords::Keywords()
:
    mPartyMembers{
        "Locklear",
        "Gorath",
        "Owyn",
        "Pug",
        "James",
        "Patrus"},
    mKeywords{}
{
    const auto& logger = Logging::LogState::GetLogger("Keywords");
    auto fb = FileBufferFactory::Get().CreateDataBuffer("KEYWORD.DAT");
    auto length = fb.GetUint16LE();
    logger.Spam() << "Loading keywords" << "\n";
    logger.Spam() << "Length: " << length << "\n";

    std::vector<unsigned> offsets{};
    unsigned i = 0;
    while (fb.Tell() != 0x2b8)
    {
        const auto& offset = offsets.emplace_back(fb.GetUint16LE());
        logger.Spam() << "I: " << i << " " 
            << std::hex << offset << std::dec << "\n";
        i++;
    }
    std::vector<std::string> strings{};
    for (auto offset : offsets)
    {
        fb.Seek(offset);
        const auto& keyword = strings.emplace_back(fb.GetString());
        mKeywords.emplace_back(keyword);
    }
    for (unsigned i = 0; i < strings.size(); i ++)
    {
        logger.Spam() << "K: " << i << " : " << strings[i] << "\n";
    }
}

std::string_view Keywords::GetDialogChoice(unsigned i) const
{
    ASSERT(i < mKeywords.size());
    return mKeywords[i];
}

std::string_view Keywords::GetQueryChoice(unsigned i) const
{
    ASSERT(i < mKeywords.size());
    return mKeywords[i];
}

std::string_view Keywords::GetNPCName(unsigned i) const
{
    ASSERT(i + mCharacterNameOffset < mKeywords.size());
    if (i < mPartyMembers.size())
    {
        ASSERT(i != 0);
        return mPartyMembers[i - 1];
    }
    return mKeywords[i + mCharacterNameOffset];
}

DialogSnippet::DialogSnippet(FileBuffer& fb, std::uint8_t dialogFile)
{
    const auto fileOffset = fb.Tell();
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
        constexpr std::uint32_t targetBit = 0xf0000000;
        if (targetBit & rawTarget)
            return KeyTarget{rawTarget & ~targetBit};
        else
            return OffsetTarget{dialogFile, rawTarget};
    };

    // DIRTY HACK BUT THE GAME IS WEIRD
    // This choice seems to be missing on the repair dialog (0x1b7763)
    // dont think this will work with anything but english translation
    if (dialogFile == 18 && fileOffset == 0x185b2)
    {
        // Manually add "CantAfford" choice here
        mChoices.emplace_back(0x7533, 0x1, 0xffff, KeyTarget{0});
    }

    for (i = 0; i < choices; i++)
    {
        const auto state   = fb.GetUint16LE();
        const auto choice0 = fb.GetUint16LE();
        const auto choice1 = fb.GetUint16LE();
        const auto offset  = fb.GetUint32LE();
        const auto target  = offset != 0 ? GetTarget(offset) : KeyTarget{0};
        mChoices.emplace_back(state, choice0, choice1, target);
    }

    if (dialogFile == 18 && fileOffset == 0x1665f)
    {
        // Manually add "Haggle" choice here
        mChoices.emplace_back(0x106, 0x1, 0xffff, KeyTarget{0});
        mChoices.emplace_back(0x105, 0x1, 0xffff, KeyTarget{0});
    }

    for (i = 0; i < actions; i++)
    {
        const auto type = fb.GetUint16LE();
        const auto dr = static_cast<DialogResult>(type);
        switch (dr)
        {
        case DialogResult::SetTextVariable:
        {
            const auto which = fb.GetUint16LE();
            const auto what = fb.GetUint16LE();
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                SetTextVariable{
                    which,
                    what,
                    rest});
        } break;
        case DialogResult::GiveItem:
        {
            const auto item = fb.GetUint8();
            const auto character = fb.GetUint8();
            const auto quantity = fb.GetUint16LE();
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                GiveItem{
                    item,
                    character,
                    quantity,
                    rest});
        } break;
        case DialogResult::LoseItem:
        {
            const auto item = fb.GetUint16LE();
            auto quantity = fb.GetUint16LE();
            if (quantity == 0)
                quantity = 1;
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                LoseItem{
                    item,
                    quantity,
                    rest});
        } break;
        case DialogResult::SetFlag:
        {
            const auto eventPtr = fb.GetUint16LE();
            const auto mask = fb.GetUint8();
            const auto data = fb.GetUint8();
            const auto zero = fb.GetUint16LE();
            const auto value = fb.GetUint16LE();
            mActions.emplace_back(
                SetFlag{
                    eventPtr,
                    mask,
                    data,
                    zero,
                    value});
        } break;
        case DialogResult::SetPopupDimensions:
        {
            const auto posX = fb.GetUint16LE();
            const auto posY = fb.GetUint16LE();
            const auto dimsX = fb.GetUint16LE();
            const auto dimsY = fb.GetUint16LE();
            mActions.emplace_back(
                SetPopupDimensions{
                    glm::vec2{posX, posY},
                    glm::vec2{dimsX, dimsY}});
        } break;
        case DialogResult::SpecialAction:
        {
            const auto type = static_cast<SpecialActionType>(fb.GetUint16LE());
            const auto var1 = fb.GetUint16LE();
            const auto var2 = fb.GetUint16LE();
            const auto var3 = fb.GetUint16LE();
            mActions.emplace_back(SpecialAction{type, var1, var2, var3});
        } break;
        case DialogResult::GainCondition:
        {
            const auto who = fb.GetUint16LE();
            const auto condition = static_cast<Condition>(fb.GetUint16LE());
            const auto min = fb.GetSint16LE();
            const auto max = fb.GetSint16LE();
            assert(min <= max);
            mActions.emplace_back(
                GainCondition{
                    who,
                    condition,
                    min,
                    max});
        } break;
        case DialogResult::GainSkill:
        {
            const auto flag = fb.GetUint16LE();
            const auto skill = static_cast<SkillType>(fb.GetUint16LE());
            const auto min = fb.GetSint16LE();
            const auto max = fb.GetSint16LE();
            assert(min <= max);
            mActions.emplace_back(
                GainSkill{flag, skill, min, max});
        } break;
        case DialogResult::LoadSkillValue:
        {
            const auto target = fb.GetUint16LE();
            const auto skill = static_cast<SkillType>(fb.GetUint16LE());
            fb.Skip(4);
            mActions.emplace_back(
                LoadSkillValue{target, skill});
        } break;
        case DialogResult::PlaySound:
        {
            const auto soundIndex = fb.GetUint16LE();
            const auto flag = fb.GetUint16LE();
            const auto rest = fb.GetUint32LE();
            mActions.emplace_back(
                PlaySound{
                    soundIndex,
                    flag,
                    rest});
        } break;
        case DialogResult::ElapseTime:
        {
            const auto time = Time{fb.GetUint32LE()};
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                ElapseTime{
                    time,
                    rest});
        } break;
        case DialogResult::SetAddResetState:
        {
            const auto state = fb.GetUint16LE();
            const auto unk0 = fb.GetUint16LE();
            const auto time = Time{fb.GetUint32LE()};
            mActions.emplace_back(
                SetAddResetState{state, unk0, time});
        } break;
        case DialogResult::PushNextDialog:
        {
            const auto offset = fb.GetUint32LE();
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                PushNextDialog{
                    GetTarget(offset),
                    rest});
        } break;
        case DialogResult::UpdateCharacters:
        {
            const auto number = fb.GetUint16LE();
            const auto char0 = CharIndex{fb.GetUint16LE()};
            const auto char1 = CharIndex{fb.GetUint16LE()};
            const auto char2 = CharIndex{fb.GetUint16LE()};
            mActions.emplace_back(
                UpdateCharacters{
                    number,
                    char0,
                    char1,
                    char2});
        } break;
        case DialogResult::HealCharacters:
        {
            const auto who = fb.GetUint16LE();
            const auto howMuch = fb.GetUint16LE();
            fb.GetUint16LE();
            fb.GetUint16LE();
            mActions.emplace_back(HealCharacters{who, howMuch});
        } break;
        case DialogResult::LearnSpell:
        {
            const auto who = fb.GetUint16LE();
            const auto whichSpell = fb.GetUint16LE();
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                LearnSpell{
                    who,
                    whichSpell});
        } break;
        case DialogResult::Teleport:
        {
            const auto teleportIndex = fb.GetUint16LE();
            fb.Skip(6);
            mActions.emplace_back(
                Teleport{TeleportIndex{teleportIndex}});
        } break;
        case DialogResult::SetEndOfDialogState:
        {
            const auto state = fb.GetSint16LE();
            const auto rest = fb.GetArray<6>();
            mActions.emplace_back(
                SetEndOfDialogState{
                    state,
                    rest});
        } break;
        case DialogResult::SetTimeExpiringState:
        {
            const auto type = ExpiringStateType{fb.GetUint8()};
            const auto flag = fb.GetUint8();
            const auto state = fb.GetUint16LE();
            const auto time = Time{fb.GetUint32LE()};
            mActions.emplace_back(
                SetTimeExpiringState{type, flag, state, time});
        } break;
        case DialogResult::LoseItem2:
        {
            const auto item = fb.GetUint16LE();
            auto quantity = fb.GetUint16LE();
            if (quantity == 0)
                quantity = 1;
            const auto rest = fb.GetArray<4>();
            mActions.emplace_back(
                LoseItem2{
                    item,
                    quantity,
                    rest});
        } break;
        default:
        {
            const auto& rest = fb.GetArray<8>();
            mActions.emplace_back(
                UnknownAction{
                    type,
                    rest});
        } break;
        }
    }
    
    if (length > 0)
        mText = fb.GetString(length);
    else
        mText = "";
}

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d)
{
    os << "[ ds: " << std::hex << +d.mDisplayStyle << " act: " << +d.mActor
        << " ds2: " << +d.mDisplayStyle2 << " ds3: " << +d.mDisplayStyle3
        << " ]" << std::dec << "\n";
    
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

const DialogStore& DialogStore::Get()
{
    static DialogStore dialogStore{};
    return dialogStore;
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
        auto fb = FileBufferFactory::Get().CreateDataBuffer(fname);
        unsigned dialogs = fb.GetUint16LE();
        mLogger.Debug() << "Dialog " << fname << " has: " << dialogs << " dialogs" << "\n";

        for (unsigned i = 0; i < dialogs; i++)
        {
            auto key = KeyTarget{fb.GetUint32LE()};
            const auto val = OffsetTarget{dialogFile, fb.GetUint32LE()};
            const auto [it, emplaced] = mDialogMap.emplace(
                key,
                val);
            //ASSERT(emplaced);
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
    mLogger.Info() << "Dialog for key: " << std::hex << dialogKey << std::dec << "\n";
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
        mLogger.Error() << __FUNCTION__ << " " << e.what() << std::endl;
        return false;
    }
}

OffsetTarget DialogStore::GetTarget(KeyTarget dialogKey) const
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

}
