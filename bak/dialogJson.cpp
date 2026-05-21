#include "bak/dialogJson.hpp"

#include "bak/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"

#include "com/json.hpp"
#include "com/logger.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace BAK::DialogJson {

namespace {

DialogResult ParseActionType(const nlohmann::json& json)
{
    if (json.is_string())
    {
        auto actionType = json.get<std::string>();
        if (actionType == "SetTextVariable") return DialogResult::SetTextVariable;
        if (actionType == "GiveItem") return DialogResult::GiveItem;
        if (actionType == "LoseItem") return DialogResult::LoseItem;
        if (actionType == "SetFlag") return DialogResult::SetFlag;
        if (actionType == "LoadActor") return DialogResult::LoadActor;
        if (actionType == "SetPopupDimensions") return DialogResult::SetPopupDimensions;
        if (actionType == "SpecialAction") return DialogResult::SpecialAction;
        if (actionType == "GainCondition") return DialogResult::GainCondition;
        if (actionType == "GainSkill") return DialogResult::GainSkill;
        if (actionType == "LoadSkillValue") return DialogResult::LoadSkillValue;
        if (actionType == "PlaySound") return DialogResult::PlaySound;
        if (actionType == "ElapseTime") return DialogResult::ElapseTime;
        if (actionType == "SetAddResetState") return DialogResult::SetAddResetState;
        if (actionType == "PushNextDialog") return DialogResult::PushNextDialog;
        if (actionType == "UpdateCharacters") return DialogResult::UpdateCharacters;
        if (actionType == "HealCharacters") return DialogResult::HealCharacters;
        if (actionType == "LearnSpell") return DialogResult::LearnSpell;
        if (actionType == "Teleport") return DialogResult::Teleport;
        if (actionType == "SetEndOfDialogState") return DialogResult::SetEndOfDialogState;
        if (actionType == "SetTimeExpiringState") return DialogResult::SetTimeExpiringState;
        if (actionType == "LoseNOfItem") return DialogResult::LoseNOfItem;
        throw std::runtime_error("Unknown action type name: " + actionType);
    }
    return static_cast<DialogResult>(json.get<std::uint16_t>());
}

template <unsigned N>
std::array<std::uint8_t, N> GetRestArray(const nlohmann::json& json)
{
    std::array<std::uint8_t, N> arr{};
    if (json.contains("rest"))
    {
        const auto& rest = json["rest"];
        for (unsigned i = 0; i < N && i < rest.size(); i++)
            arr[i] = rest[i].get<std::uint8_t>();
    }
    return arr;
}

Target ParseTarget(const nlohmann::json& json)
{
    if (json.contains("key"))
        return KeyTarget{json["key"].get<std::uint32_t>()};
    if (json.contains("offset"))
        return OffsetTarget{
            static_cast<std::uint8_t>(json["offset"]["file"].get<unsigned>()),
            json["offset"]["offset"].get<std::uint32_t>()};
    throw std::runtime_error("Target must have 'key' or 'offset'");
}

DialogAction ParseAction(const nlohmann::json& json)
{
    const auto type = ParseActionType(json["type"]);
    switch (type)
    {
    case DialogResult::SetTextVariable:
        return SetTextVariable{
            json["which"].get<std::uint16_t>(),
            json["what"].get<std::uint16_t>(),
            GetRestArray<4>(json)};
    case DialogResult::GiveItem:
        return GiveItem{
            static_cast<std::uint8_t>(json["itemIndex"].get<unsigned>()),
            static_cast<std::uint8_t>(json.value("who", 0)),
            json["quantity"].get<std::uint16_t>(),
            GetRestArray<4>(json)};
    case DialogResult::LoseItem:
        return LoseItem{
            json["itemIndex"].get<std::uint16_t>(),
            json["quantity"].get<std::uint16_t>(),
            GetRestArray<4>(json)};
    case DialogResult::SetFlag:
        return SetFlag{
            json["eventPointer"].get<std::uint16_t>(),
            static_cast<std::uint8_t>(json["mask"].get<unsigned>()),
            static_cast<std::uint8_t>(json["data"].get<unsigned>()),
            json["alwaysZero"].get<std::uint16_t>(),
            json["eventValue"].get<std::uint16_t>()};
    case DialogResult::LoadActor:
        return LoadActor{
            json["actor1"].get<std::uint16_t>(),
            json["actor2"].get<std::uint16_t>(),
            json["actor3"].get<std::uint16_t>(),
            json["unknown"].get<std::uint16_t>()};
    case DialogResult::SetPopupDimensions:
        return SetPopupDimensions{
            glm::vec2{
                json["posX"].get<std::uint16_t>(),
                json["posY"].get<std::uint16_t>()},
            glm::vec2{
                json["dimsX"].get<std::uint16_t>(),
                json["dimsY"].get<std::uint16_t>()}};
    case DialogResult::SpecialAction:
        return SpecialAction{
            static_cast<SpecialActionType>(json["specialType"].get<std::uint16_t>()),
            json["var1"].get<std::uint16_t>(),
            json["var2"].get<std::uint16_t>(),
            json["var3"].get<std::uint16_t>()};
    case DialogResult::GainCondition:
        return GainCondition{
            json["who"].get<std::uint16_t>(),
            static_cast<Condition>(json["condition"].get<std::uint16_t>()),
            json["min"].get<std::int16_t>(),
            json["max"].get<std::int16_t>()};
    case DialogResult::GainSkill:
        return GainSkill{
            json["who"].get<std::uint16_t>(),
            static_cast<SkillType>(json["skill"].get<std::uint16_t>()),
            json["min"].get<std::int16_t>(),
            json["max"].get<std::int16_t>()};
    case DialogResult::LoadSkillValue:
        return LoadSkillValue{
            json["target"].get<std::uint16_t>(),
            static_cast<SkillType>(json["skill"].get<std::uint16_t>())};
    case DialogResult::PlaySound:
        return PlaySound{
            json["soundIndex"].get<std::uint16_t>(),
            json["flag"].get<std::uint16_t>(),
            json.value("rest", 0u)};
    case DialogResult::ElapseTime:
        return ElapseTime{
            Time{json["time"].get<std::uint32_t>()},
            GetRestArray<4>(json)};
    case DialogResult::SetAddResetState:
        return SetAddResetState{
            json["eventPtr"].get<std::uint16_t>(),
            json["unknown0"].get<std::uint16_t>(),
            Time{json["timeToExpire"].get<std::uint32_t>()}};
    case DialogResult::PushNextDialog:
        return PushNextDialog{
            ParseTarget(json["target"]),
            GetRestArray<4>(json)};
    case DialogResult::UpdateCharacters:
    {
        std::vector<CharIndex> chars;
        for (const auto& charEntry : json["characters"])
            chars.emplace_back(charEntry.get<std::uint16_t>());
        return UpdateCharacters{chars};
    }
    case DialogResult::HealCharacters:
        return HealCharacters{
            json["who"].get<std::uint16_t>(),
            json["howMuch"].get<std::uint16_t>()};
    case DialogResult::LearnSpell:
        return LearnSpell{
            json["who"].get<std::uint16_t>(),
            json["whichSpell"].get<std::uint16_t>()};
    case DialogResult::Teleport:
        return Teleport{
            TeleportIndex{json["teleportIndex"].get<std::uint16_t>()}};
    case DialogResult::SetEndOfDialogState:
        return SetEndOfDialogState{
            json["state"].get<std::int16_t>(),
            GetRestArray<6>(json)};
    case DialogResult::SetTimeExpiringState:
        return SetTimeExpiringState{
            ExpiringStateType{static_cast<std::uint8_t>(json["expiringType"].get<unsigned>())},
            static_cast<std::uint8_t>(json["flags"].get<unsigned>()),
            json["eventPtr"].get<std::uint16_t>(),
            Time{json["timeToExpire"].get<std::uint32_t>()}};
    case DialogResult::LoseNOfItem:
        return LoseNOfItem{
            json["itemIndex"].get<std::uint16_t>(),
            json["quantity"].get<std::uint16_t>(),
            GetRestArray<4>(json)};
    default:
        return UnknownAction{
            static_cast<std::uint16_t>(type),
            GetRestArray<8>(json)};
    }
}

DialogSnippet ParseSnippet(const nlohmann::json& json)
{
    DialogSnippet snippet;
    snippet.mDisplayStyle = static_cast<std::uint8_t>(json["displayStyle"].get<unsigned>());
    snippet.mActor = json.value("actor", 0);
    snippet.mDisplayStyle2 = static_cast<std::uint8_t>(json.value("displayStyle2", 0));
    snippet.mDisplayStyle3 = static_cast<std::uint8_t>(json.value("displayStyle3", 0));
    snippet.mText = json.value("text", "");

    if (json.contains("actions"))
    {
        for (const auto& actionJson : json["actions"])
            snippet.mActions.push_back(ParseAction(actionJson));
    }

    if (json.contains("choices"))
    {
        for (const auto& choiceJson : json["choices"])
        {
            auto state = choiceJson["state"].get<std::uint16_t>();
            auto min = choiceJson.value("min", 0);
            auto max = choiceJson.value("max", 0);
            auto target = ParseTarget(choiceJson["target"]);
            snippet.mChoices.emplace_back(state, min, max, target);
        }
    }

    return snippet;
}

}

void LoadAllFromDirectory(const std::string& dir)
{
    auto& log = Logging::LogState::GetLogger("DialogJson");

    if (!std::filesystem::exists(dir))
    {
        log.Debug() << "Dialog override directory not found: " << dir << "\n";
        return;
    }

    log.Info() << "Loading dialog overrides from: " << dir << "\n";

    auto& store = DialogStore::Get();
    const auto callback = nullptr;
    const bool allow_exceptions = true;
    const bool ignore_comments = true;

    unsigned fileCount = 0;
    unsigned dialogCount = 0;

    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        fileCount++;

        std::ifstream in{entry.path()};
        auto data = nlohmann::json::parse(in, callback, allow_exceptions, ignore_comments);

        unsigned injected = 0;
        for (const auto& dialog : data["dialogs"])
        {
            auto key = dialog["key"].get<std::uint32_t>();
            auto snippet = ParseSnippet(dialog["snippet"]);
            store.InjectDialog(KeyTarget{key}, std::move(snippet));
            injected++;
        }

        log.Info() << "  Loaded " << injected << " dialogs from "
            << entry.path().filename().string() << "\n";
        dialogCount += injected;
    }

    log.Info() << "Loaded " << dialogCount << " total dialogs from "
        << fileCount << " file(s) in " << dir << "\n";
}

}
