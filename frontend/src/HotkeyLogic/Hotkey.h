#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <set>

struct HotkeyCombo {
    int keycode;
    bool ctrl;
    bool alt;
    bool shift;
    bool system; //windows

    bool operator==(const HotkeyCombo& otherHotKeys) const;
};

class Hotkey {
    public:
        Hotkey (const std::string& id, const HotkeyCombo& hotkeys, std::function<void()> callback, HWND window);
        ~Hotkey();

        Hotkey(const Hotkey&) = delete;
        Hotkey& operator=(const Hotkey&) = delete;

        std::string getId() const;
        int getsystemID() const;
        HotkeyCombo getHotkeys() const;

        void setID(const std::string& id) const;
        void setsystemID() const;
        void setCallback(std::function<void()> callback) const;

        bool registerHotkeys();
        void unregisterHotkeys();
        bool isRegistered() const;
        void triggerCallback() const;

    private:
        std::string id;
        int systemID;
        HotkeyCombo hotkeys;
        std::function<void()> callback;
        HWND window;
        bool registered;

        static const int SYSTEM_ID_LIMIT;
        static int nextsystemID;
        static std::set<int> freedSystemIDs;
};