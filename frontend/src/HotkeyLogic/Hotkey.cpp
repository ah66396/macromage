#include <Hotkey.h>

bool HotkeyCombo::operator==(const HotkeyCombo& o) const {
    return (keycode == o.keycode) && (ctrl == o.ctrl) && (alt == o.alt) && (shift == o.shift) && (system == o.system);
};

Hotkey::Hotkey(const std::string& id, const HotkeyCombo& hotkeys, std::function<void()> callback, HWND window)
    : id{id},
      systemID{-1},
      hotkeys{hotkeys},
      callback{callback},
      window{window},
      registered(false)
{};

Hotkey::~Hotkey() {
    unregisterHotkeys();
};

bool Hotkey::registerHotkeys() {
    if (registered) {
        return true;
    }
    
    int assignedID = -1;
    if (!freedSystemIDs.empty()) {
        auto p = freedSystemIDs.begin();
        assignedID = *p;
        freedSystemIDs.erase(p);
    }

    

    
};