#pragma once

#include "audio/audio.hpp"

#include "com/string.hpp"

#include "imgui/imgui.h"

#include <iomanip>

class Camera;
namespace BAK {
class GameState;
}

namespace Gui {
class IGuiManager;
}

namespace Game {
class GameRunner;
}

struct Console : public std::streambuf
{
    using ConsoleCommand = std::function<void(const std::vector<std::string>&)>;

    std::streamsize xsputn(const char_type* s, std::streamsize n);
    int_type overflow(int_type c);
    
    // Console commands
    void ShowTeleports(const std::vector<std::string>& words);
    void NextChapter(const std::vector<std::string>& words);
    void SetComplexEvent(const std::vector<std::string>& words);
    void CastSpell(const std::vector<std::string>& words);
    void SaveGame(const std::vector<std::string>& words);
    void StopMusic(const std::vector<std::string>& words);
    void SwitchMidiPlayer(const std::vector<std::string>& words);
    void PlayMusic(const std::vector<std::string>& words);
    void LoadGame(const std::vector<std::string>& words);
    void PlaySound(const std::vector<std::string>& words);
    void SetEventState(const std::vector<std::string>& words);
    void RemoveItem(const std::vector<std::string>& words);
    void GiveItem(const std::vector<std::string>& words);
    void DoTeleport(const std::vector<std::string>& words);
    void SetPosition(const std::vector<std::string>& words);
    void SetChapter(const std::vector<std::string>& words);
    void SetLogLevel(const std::vector<std::string>& words);
    
    void DisableLogger(const std::vector<std::string>& words);
    Console();
    ~Console();
    
    void ToggleLog();
    void ClearLog();
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void Draw(const char* title, bool* p_open);
    void ExecCommand(const char* command_line);
    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);
    int TextEditCallback(ImGuiInputTextCallbackData* data);

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

static void ShowConsole(bool* p_open);

