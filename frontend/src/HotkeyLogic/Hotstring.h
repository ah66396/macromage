#pragma once
#include <Windows.h>
#include <string>
#include <functional>
#include <chrono>
#include <array>
#include <vector>
#include <cstdint>

struct HotstringCombo {
    std::string trigger;
    bool ctrl;
    bool alt;
    bool shift;
    bool system;
};

class Hotstring {
public:
    Hotstring(const std::string& name,
              std::array<bool, 4> mods,
              const std::string& trigger,
              std::function<void()> callback,
              int64_t timeThresholdMs = 2000);
    ~Hotstring();
    
    bool processKeyPress(int vkCode, DWORD scanCode, DWORD flags);
    bool isObserving() const { return observing; }
    
private:
    struct BufferedKey {
        int vkCode;
        DWORD scanCode;
        DWORD flags;
    };
    
    void startObservation(int vkCode, DWORD scanCode, DWORD flags);
    void continueObservation(int vkCode, DWORD scanCode, DWORD flags);
    void failObservation();
    void succeedObservation();
    void resendBufferedKeys();

    bool isTimeExpired() const;

    bool checkModifiers() const;
    
    char vkToChar(int vkCode) const;
    
    std::string name;
    HotstringCombo hotstring;
    std::function<void()> callback;
    
    bool observing;
    std::string currentBuffer;
    std::vector<BufferedKey> bufferedKeys;
    std::chrono::steady_clock::time_point observationStartTime;
    int64_t timeThresholdMs;
    
    HHOOK keyboardHook;
    static Hotstring* instance;
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
};