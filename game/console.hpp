#pragma once

#include "audio/audio.hpp"

#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "bak/spells.hpp"
#include "bak/worldClock.hpp"

#include "com/string.hpp"

#include "game/gameRunner.hpp"

#include "imgui/imgui.h"

#include <iomanip>

struct Console : public std::streambuf
{
    using ConsoleCommand = std::function<void(const std::vector<std::string>&)>;

    std::streamsize xsputn(const char_type* s, std::streamsize n)
    {
        std::size_t from = 0;
        for (unsigned i = 0; i < n; i++)
        {
            if (s[i] == '\n')
            {
                mStreamBuffer += std::string{s + from, i - from};
                AddLog(mStreamBuffer.c_str());
                mStreamBuffer.clear();
                from = i;
            }
        }

        if (from != static_cast<std::size_t>(n))
        {
            mStreamBuffer += std::string{s + from, n - from};
        }

        return n;
    }

    int_type overflow(int_type c)
    {
        mStreamBuffer += std::string{1, static_cast<char_type>(c)};
        if (c == '\n')
        {
            AddLog(mStreamBuffer.c_str());
            mStreamBuffer.clear();
        }
        return c;
    }

    // Console commands
    void ShowTeleports(const std::vector<std::string>& words)
    {
        if (!mGameRunner)
        {
            AddLog("[error] SHOW_TELEPORTS FAILED No GameRunner Connected");
            return;
        }

        std::stringstream ss{};
        for (unsigned i = 0; i < 40; i++)
        {
            ss = std::stringstream{};
            ss << i << " " << mGameRunner->mTeleportFactory.Get(i);
            AddLog(ss.str().c_str());
        }
    }

    void SetComplexEvent(const std::vector<std::string>& words)
    {
        if (words.size() < 5)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SET_COMPLEX_EVENT EV_PTR EV_MASK EV_DATA EV_VALUE (%s)", ss.str().c_str());
            return;
        }

        if (!mGameState)
        {
            AddLog("[error] SET_COMPLEX_EVENT FAILED No GameState Connected");
            return;
        }

        std::uint16_t eventPtr;
        {
            std::stringstream ss{};
            ss << words[1];
            ss >> eventPtr;
        }
        
        unsigned eventMask;
        {
            std::stringstream ss{};
            ss << words[2];
            ss >> eventMask;
        }

        unsigned eventData;
        {
            std::stringstream ss{};
            ss << words[3];
            ss >> eventData;
        }

        std::uint16_t eventValue;
        {
            std::stringstream ss{};
            ss << words[4];
            ss >> eventValue;
        }

        auto setFlag = BAK::SetFlag{
            eventPtr,
            static_cast<std::uint8_t>(eventMask),
            static_cast<std::uint8_t>(eventData),
            0,
            eventValue};
        Logging::LogInfo(__FUNCTION__) << " Setting event with: " << setFlag << "\n";
        mGameState->SetEventState(setFlag);
    }

    void CastSpell(const std::vector<std::string>& words)
    {
        if (words.size() < 3)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: CAST_SPELL SPELL DURATION (%s)", ss.str().c_str());
            return;
        }

        if (!mGameState)
        {
            AddLog("[error] CAST_SPELL FAILED No GameState Connected");
            return;
        }

        std::uint16_t spellI;
        {
            std::stringstream ss{};
            ss << words[1];
            ss >> spellI;
        }
        auto spell = BAK::StaticSpells{spellI};
        std::uint32_t duration;
        {
            std::stringstream ss{};
            ss << words[2];
            ss >> duration;
        }

        mGameState->CastStaticSpell(spell, BAK::Time{duration});
    }
     
    void SaveGame(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SAVE_GAME FILENAME (%s)", ss.str().c_str());
            return;
        }

        if (!mGameState)
        {
            AddLog("[error] SAVE_GAME FAILED No GameState Connected");
            return;
        }

        const auto saved = mGameState->Save(words[1]);
        if (saved)
            AddLog("Game saved to: %s", words[1].c_str());
        else
            AddLog("Game not saved, no game data");

    }

    void StopMusic(const std::vector<std::string>& words)
    {
        AudioA::AudioManager::Get().StopMusicTrack();
    }

    void SwitchMidiPlayer(const std::vector<std::string>& words)
    {
        if (words.size() < 1)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SWITCH_MIDI_PLAYER MIDI_PLAYER [0, 1, 2] (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned midiPlayer;
        ss >> std::setbase(0) >> midiPlayer;

        AudioA::AudioManager::Get().SwitchMidiPlayer(static_cast<AudioA::MidiPlayer>(midiPlayer));
    }

    void PlayMusic(const std::vector<std::string>& words)
    {
        if (words.size() < 1)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: PLAY_MUSIC MUSIC_INDEX (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned music_index;
        ss >> std::setbase(0) >> music_index;

        AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{music_index});
    }

    void LoadGame(const std::vector<std::string>& words)
    {
        if (words.size() < 1)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: LOAD_GAME SAVE_PATH (%s)", ss.str().c_str());
            return;
        }

        ASSERT(mGameRunner);
        mGameRunner->LoadGame(words[1], std::nullopt);
        mGuiManager->EnterMainView();
    }

    void PlaySound(const std::vector<std::string>& words)
    {
        if (words.size() < 1)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: PLAY_SOUND SOUND_INDEX (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned sound_index;
        ss >> std::setbase(0) >> sound_index;

        AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{sound_index});
    }

    void SetEventState(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SET_EVENT_STATE PTR VALUE (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned ptr;
        ss >> std::setbase(0) >> ptr;
        ss = std::stringstream{};
        ss << words[2];
        unsigned value;
        ss >> std::setbase(0) >> value;

        if (!mGameState)
        {
            AddLog("[error] SET_EVENT_STATE FAILED No GameState Connected");
            return;
        }

        mGameState->SetEventValue(ptr, value);
    }

    void RemoveItem(const std::vector<std::string>& words)
    {
        if (words.size() < 3)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: REMOVE_ITEM INDEX AMOUNT (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned itemIndex;
        ss >> std::setbase(0) >> itemIndex;
        ss = std::stringstream{};
        ss << words[2];
        unsigned amount;
        ss >> std::setbase(0) >> amount;

        if (!mGameState)
        {
            AddLog("[error] REMOVE_ITEM FAILED No GameState Connected");
            return;
        }

        mGameState->GetParty().RemoveItem(
            itemIndex,
            amount);
    }
    void GiveItem(const std::vector<std::string>& words)
    {
        if (words.size() < 3)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: GIVE_ITEM INDEX AMOUNT (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned itemIndex;
        ss >> std::setbase(0) >> itemIndex;
        ss = std::stringstream{};
        ss << words[2];
        unsigned amount;
        ss >> std::setbase(0) >> amount;

        if (!mGameState)
        {
            AddLog("[error] GiveItem FAILED No GameState Connected");
            return;
        }

        mGameState->GetParty().GainItem(
            itemIndex,
            amount);
    }

    void DoTeleport(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: DO_TELEPORT INDEX (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned index;
        ss >> std::setbase(0) >> index;

        if (!mGameRunner)
        {
            AddLog("[error] DoTeleport FAILED No GameRunner Connected");
            return;
        }

        mGameRunner->DoTeleport(BAK::TeleportIndex{index});
    }

    void SetPosition(const std::vector<std::string>& words)
    {
        if (words.size() < 3)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SET_POSITION X Y (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned x;
        ss >> std::setbase(0) >> x;
        ss = std::stringstream{};
        ss << words[2];
        unsigned y;
        ss >> std::setbase(0) >> y;

        if (!mCamera)
        {
            AddLog("[error] SetPosition FAILED No Camera Connected");
            return;
        }

        AddLog("SetPosition(%d, %d", x, y);
        mCamera->SetGameLocation(
            BAK::GamePositionAndHeading{
                BAK::GamePosition{x, y},
                BAK::GameHeading{0}});
    }

    void SetChapter(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SET_CHAPTER CHAPTER (%s)", ss.str().c_str());
            return;
        }

        std::stringstream ss{};
        ss << words[1];
        unsigned chapter;
        ss >> std::setbase(0) >> chapter;

        if (!mGameState)
        {
            AddLog("[error] SetChapter FAILED No GameState Connected");
            return;
        }

        mGameState->SetChapter(BAK::Chapter{chapter});
    }

    void SetLogLevel(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: SET_LOG_LEVEL LEVEL (%s)", ss.str().c_str());
            return;
        }

        if (words[1] == LevelToString(Logging::LogLevel::Spam))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Spam);
        }
        else if (words[1] == LevelToString(Logging::LogLevel::Debug))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Debug);
        }
        else if (words[1] == LevelToString(Logging::LogLevel::Info))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Info);
        }
        else if (words[1] == LevelToString(Logging::LogLevel::Fatal))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Fatal);
        }
        else
        {
            AddLog("[error] SET_LOG_LEVEL FAILED Invalid log level provided");
        }
    }

    void DisableLogger(const std::vector<std::string>& words)
    {
        if (words.size() < 2)
        {
            std::stringstream ss{};
            for (const auto& w : words)
                ss << w << "|";
            AddLog("[error] Usage: DISABLE_LOGGER LOGGER (%s)", ss.str().c_str());
            return;
        }

        Logging::LogState::Disable(words[1]);
    }

    Console()
    :
        mStream{this},
        mStreamBuffer{}
    {
        ClearLog();
        memset(mInputBuf, 0, sizeof(mInputBuf));
        mHistoryPos = -1;

        mCommands.push_back("HELP");
        mCommandActions.emplace_back([this](const auto&)
        {
            AddLog("Commands:");
            for (int i = 0; i < mCommands.Size; i++)
                AddLog("- %s", mCommands[i]);
        });

        mCommands.push_back("SAVE_GAME");
        mCommandActions.emplace_back([this](const auto& cmd){ SaveGame(cmd); });
        mCommands.push_back("SET_EVENT_STATE");
        mCommandActions.emplace_back([this](const auto& cmd){ SetEventState(cmd); });
        mCommands.push_back("REMOVE_ITEM");
        mCommandActions.emplace_back([this](const auto& cmd){ RemoveItem(cmd); });
        mCommands.push_back("GIVE_ITEM");
        mCommandActions.emplace_back([this](const auto& cmd){ GiveItem(cmd); });
        mCommands.push_back("DO_TELEPORT");
        mCommandActions.emplace_back([this](const auto& cmd){ DoTeleport(cmd); });

        mCommands.push_back("SHOW_TELEPORTS");
        mCommandActions.emplace_back([this](const auto& cmd){ ShowTeleports(cmd); });
        mCommands.push_back("SET_POSITION");
        mCommandActions.emplace_back([this](const auto& cmd){ SetPosition(cmd); });

        mCommands.push_back("SET_CHAPTER");
        mCommandActions.emplace_back([this](const auto& cmd){ SetChapter(cmd); });

        mCommands.push_back("STOP_MUSIC");
        mCommandActions.emplace_back([this](const auto& cmd){ StopMusic(cmd); });

        mCommands.push_back("SWITCH_MIDI_PLAYER");
        mCommandActions.emplace_back([this](const auto& cmd){ SwitchMidiPlayer(cmd); });

        mCommands.push_back("LOAD_GAME");
        mCommandActions.emplace_back([this](const auto& cmd){ LoadGame(cmd); });

        mCommands.push_back("PLAY_SOUND");
        mCommandActions.emplace_back([this](const auto& cmd){ PlaySound(cmd); });

        mCommands.push_back("PLAY_MUSIC");
        mCommandActions.emplace_back([this](const auto& cmd){ PlayMusic(cmd); });

        mCommands.push_back("HISTORY");
        mCommandActions.emplace_back([this](const auto& cmd)
        {
            int first = mHistory.Size - 10;
            for (int i = first > 0 ? first : 0; i < mHistory.Size; i++)
                AddLog("%3d: %s\n", i, mHistory[i]);
        });

        mCommands.push_back("SET_LOG_LEVEL");
        mCommandActions.emplace_back([this](const auto& cmd){ SetLogLevel(cmd); });

        mCommands.push_back("DISABLE_LOGGER");
        mCommandActions.emplace_back([this](const auto& cmd){ DisableLogger(cmd); });

        mCommands.push_back("SET_COMPLEX_EVENT");
        mCommandActions.emplace_back([this](const auto& cmd){ SetComplexEvent(cmd); });

        mCommands.push_back("CAST_SPELL");
        mCommandActions.emplace_back([this](const auto& cmd){ CastSpell(cmd); });

        mCommands.push_back("CLEAR");
        mCommandActions.emplace_back([this](const auto& cmd)
        {
            ClearLog();
        });

        ASSERT(static_cast<std::size_t>(mCommands.size())
            == mCommandActions.size());
        mAutoScroll = true;
        mScrollToBottom = false;
        AddLog("Enter HELP for a list of commands");

        mStreamLog = false;
    }

    ~Console()
    {
        ClearLog();
        for (int i = 0; i < mHistory.Size; i++)
            free(mHistory[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void ToggleLog()
    {
        mStreamLog = !mStreamLog;
        if (mStreamLog)
            Logging::LogState::AddStream(&mStream);
        else
            Logging::LogState::RemoveStream(&mStream);
    }


    void ClearLog()
    {
        for (int i = 0; i < mItems.Size; i++)
            free(mItems[i]);
        mItems.clear();
    }

    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        mItems.push_back(Strdup(buf));
    }

    void Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped("Enter 'HELP' for help.");

        if (ImGui::SmallButton(mStreamLog ? "Logging On" : "Logging Off")) { ToggleLog(); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear")) { ClearLog(); }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &mAutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        mFilter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        // Display every line as a separate entry so we can change their color or add custom widgets.
        // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
        // to only process visible items. The clipper will automatically measure the height of your first item and then
        // "seek" to display only items in the visible area.
        // To use the clipper we can replace your standard loop:
        //      for (int i = 0; i < mItems.Size; i++)
        //   With:
        //      ImGuiListClipper clipper;
        //      clipper.Begin(mItems.Size);
        //      while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // - That your items are evenly spaced (same height)
        // - That you have cheap random access to your elements (you can access them given their index,
        //   without processing all the ones before)
        // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
        // We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
        // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
        // to improve this example code!
        // If your items are of variable height:
        // - Split them into same height items would be simpler and facilitate random-seeking into your list.
        // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        for (int i = 0; i < mItems.Size; i++)
            ImGui::TextUnformatted(mItems[i]);
        //for (int i = 0; i < mItems.Size; i++)
        //{
        //    const char* item = mItems[i];
        //    if (!mFilter.PassFilter(item))
        //        continue;

        //    // Normally you would store more information in your item than just a string.
        //    // (e.g. make mItems[] an array of structure, store color/type etc.)
        //    ImVec4 color;
        //    bool has_color = false;
        //    if (strstr(item, "[error]"))          { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
        //    else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
        //    if (has_color)
        //        ImGui::PushStyleColor(ImGuiCol_Text, color);
        //    ImGui::TextUnformatted(item);
        //    if (has_color)
        //        ImGui::PopStyleColor();
        //}
        if (copy_to_clipboard)
            ImGui::LogFinish();

        if (mScrollToBottom || (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        mScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags 
            = ImGuiInputTextFlags_EnterReturnsTrue 
            | ImGuiInputTextFlags_CallbackCompletion 
            | ImGuiInputTextFlags_CallbackHistory;

        const auto inputText = ImGui::InputText(
            "Input",
            mInputBuf,
            IM_ARRAYSIZE(mInputBuf),
            input_text_flags,
            &TextEditCallbackStub,
            (void*)this);

        if (inputText)
        {
            char* s = mInputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        mHistoryPos = -1;
        for (int i = mHistory.Size - 1; i >= 0; i--)
            if (Stricmp(mHistory[i], command_line) == 0)
            {
                free(mHistory[i]);
                mHistory.erase(mHistory.begin() + i);
                break;
            }
        mHistory.push_back(Strdup(command_line));

        const auto command = std::string{command_line};
        const auto words = SplitString(" ", command);

        bool succeeded = false;
        for (unsigned i = 0; i < mCommandActions.size(); i++)
        {
            const auto candidate = std::string{mCommands[i]};
            if (command.substr(0, candidate.length()) == candidate)
            {
                std::invoke(mCommandActions[i], words);
                succeeded = true;
            }
        }

        if (!succeeded) 
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }

        // On command input, we scroll to bottom even if mAutoScroll==false
        mScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        Console* console = (Console*)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < mCommands.Size; i++)
                    if (Strnicmp(mCommands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(mCommands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = mHistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (mHistoryPos == -1)
                        mHistoryPos = mHistory.Size - 1;
                    else if (mHistoryPos > 0)
                        mHistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (mHistoryPos != -1)
                        if (++mHistoryPos >= mHistory.Size)
                            mHistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != mHistoryPos)
                {
                    const char* history_str = (mHistoryPos >= 0) ? mHistory[mHistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }

//private:
    char                  mInputBuf[256];
    ImVector<char*>       mItems;
    ImVector<const char*> mCommands;
    std::vector<ConsoleCommand> mCommandActions;
    ImVector<char*>       mHistory;
    int                   mHistoryPos;    // -1: new line, 0..mHistory.Size-1 browsing history.
    ImGuiTextFilter       mFilter;
    bool                  mAutoScroll;
    bool                  mScrollToBottom;
    bool                  mStreamLog;
    std::ostream          mStream;
    std::string mStreamBuffer;

    Camera*  mCamera;
    Game::GameRunner*  mGameRunner;
    Gui::IGuiManager*  mGuiManager;
    BAK::GameState*  mGameState;
};

static void ShowConsole(bool* p_open)
{
    static Console console;
    console.Draw("Console", p_open);
}

