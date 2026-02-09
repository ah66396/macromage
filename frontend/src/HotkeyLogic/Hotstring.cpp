#include "Hotstring.h"
#include <cctype>

Hotstring* Hotstring::instance = nullptr;

Hotstring::Hotstring(const std::string& name,
                     std::array<bool, 4> mods,
                     const std::string& trigger,
                     std::function<void()> callback,
                     int64_t timeThresholdMs)
    : name(name),
      hotstring{trigger, mods[0], mods[1], mods[2], mods[3]},
      callback(callback),
      observing(false),
      timeThresholdMs(timeThresholdMs),
      keyboardHook(nullptr)
{
    instance = this;
    
    for (char& c : hotstring.trigger) {
        c = std::tolower(c);
    }
    
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 
                                     GetModuleHandle(nullptr), 0);
}

Hotstring::~Hotstring() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
    instance = nullptr;
}

bool Hotstring::checkModifiers() const {
    bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; // 0x8000: high-order bit indicates key down
    bool altPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    bool systemPressed = (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0 || 
                         (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;
    
    return (ctrlPressed == hotstring.ctrl) &&
           (altPressed == hotstring.alt) &&
           (shiftPressed == hotstring.shift) &&
           (systemPressed == hotstring.system);
}

char Hotstring::vkToChar(int vkCode) const {
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);
    
    WORD result;
    if (ToAscii(vkCode, MapVirtualKey(vkCode, MAPVK_VK_TO_VSC), keyboardState, &result, 0) == 1) {
        return static_cast<char>(result);
    }
    return '\0';
}

bool Hotstring::processKeyPress(int vkCode, DWORD scanCode, DWORD flags) {
    // 0x80: key release
    if (flags & 0x80) { 
        return false;
    }
    
    if (vkCode == VK_CONTROL || vkCode == VK_MENU || 
        vkCode == VK_SHIFT || vkCode == VK_LWIN || vkCode == VK_RWIN) {
        return false;
    }
    
    if (observing && isTimeExpired()) {
        failObservation();
    }
    
    char c = vkToChar(vkCode);
    if (c == '\0' || !std::isalpha(c)) {
        if (observing) {
            failObservation();
        }
        return false;
    }
    
    char lowerC = std::tolower(c);
    
    if (!observing) {
        if (checkModifiers() && lowerC == hotstring.trigger[0]) {
            startObservation(vkCode, scanCode, flags);
            return true;
        }
        return false;
    } else {
        if (!checkModifiers()) {
            failObservation();
            return false;
        }
        
        continueObservation(vkCode, scanCode, flags);
        return true;
    }
}

void Hotstring::startObservation(int vkCode, DWORD scanCode, DWORD flags) {
    observing = true;
    currentBuffer.clear();
    bufferedKeys.clear();
    observationStartTime = std::chrono::steady_clock::now();
    
    char c = vkToChar(vkCode);
    currentBuffer += std::tolower(c);
    bufferedKeys.push_back({vkCode, scanCode, flags});
    
    if (currentBuffer == hotstring.trigger) {
        succeedObservation();
    }
}

void Hotstring::continueObservation(int vkCode, DWORD scanCode, DWORD flags) {
    char c = vkToChar(vkCode);
    currentBuffer += std::tolower(c);
    bufferedKeys.push_back({vkCode, scanCode, flags});
    
    if (currentBuffer.length() > hotstring.trigger.length()) {
        failObservation();
        return;
    }
    
    if (currentBuffer != hotstring.trigger.substr(0, currentBuffer.length())) {
        failObservation();
        return;
    }
    
    if (currentBuffer == hotstring.trigger) {
        succeedObservation();
        return;
    }
}

void Hotstring::failObservation() {
    resendBufferedKeys();
    observing = false;
    currentBuffer.clear();
    bufferedKeys.clear();
}

void Hotstring::succeedObservation() {
    observing = false;
    currentBuffer.clear();
    bufferedKeys.clear();
    
    if (callback) {
        callback();
    }
}

void Hotstring::resendBufferedKeys() {
    std::vector<INPUT> inputs;
    inputs.reserve(bufferedKeys.size() * 2);
    
    for (const auto& key : bufferedKeys) {
        INPUT inputDown = {0};
        inputDown.type = INPUT_KEYBOARD;
        inputDown.ki.wVk = key.vkCode;
        inputDown.ki.wScan = key.scanCode;
        inputDown.ki.dwFlags = 0;
        inputs.push_back(inputDown);
        
        INPUT inputUp = {0};
        inputUp.type = INPUT_KEYBOARD;
        inputUp.ki.wVk = key.vkCode;
        inputUp.ki.wScan = key.scanCode;
        inputUp.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(inputUp);
    }
    
    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    }
}

bool Hotstring::isTimeExpired() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - observationStartTime).count();
    return elapsed > timeThresholdMs;
}

LRESULT CALLBACK Hotstring::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            
            bool consumed = instance->processKeyPress(
                pKbd->vkCode, 
                pKbd->scanCode, 
                pKbd->flags
            );
            
            if (consumed) {
                return 1;
            }
        }
    }
    
    return CallNextHookEx(instance->keyboardHook, nCode, wParam, lParam);
}