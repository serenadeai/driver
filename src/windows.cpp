#include <windows.h>

// must be included after <windows.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <winuser.h>

#include <algorithm>
#include <tuple>
#include <vector>

#include "windows.hpp"

namespace driver {

void Click(const std::string& button, int count) {
  for (int i = 0; i < count; i++) {
    MouseDown(button);
    MouseUp(button);
  }
}

void FocusApplication(const std::string& application) {
  EnumWindows(FocusWindow, reinterpret_cast<LPARAM>(&application));
}

BOOL CALLBACK FocusWindow(HWND window, LPARAM data) {
  std::string& name = *reinterpret_cast<std::string*>(data);
  std::string process = ProcessName(window);
  if (process.find(name) != std::string::npos) {
    // ignore invisible windows
    RECT rect;
    GetWindowRect(window, &rect);
    if (rect.top == rect.bottom || rect.left == rect.right ||
        !IsWindowVisible(window)) {
      return TRUE;
    }

    // in order to switch window focus, the alt key needs to be pressed down,
    // for some reason
    ToggleKey("alt", true);
    WINDOWPLACEMENT placement;
    GetWindowPlacement(window, &placement);
    if (placement.showCmd == SW_SHOWMAXIMIZED) {
      ShowWindow(window, SW_SHOWMAXIMIZED);
    } else if (placement.showCmd == SW_SHOWMINIMIZED) {
      ShowWindow(window, SW_RESTORE);
    } else {
      ShowWindow(window, SW_NORMAL);
    }

    AllowSetForegroundWindow(ASFW_ANY);
    SetForegroundWindow(window);
    ToggleKey("alt", false);
    return FALSE;
  }

  return TRUE;
}

std::string GetActiveApplication() {
  HWND active = GetForegroundWindow();
  return ProcessName(active);
}

std::tuple<int, int> GetMouseLocation() {
  POINT point;
  GetCursorPos(&point);

  std::tuple<int, int> result;
  std::get<0>(result) = point.x;
  std::get<1>(result) = point.y;
  return result;
}

std::vector<std::string> GetRunningApplications() {
  std::vector<std::string> result;
  EnumWindows(GetRunningWindows, reinterpret_cast<LPARAM>(&result));

  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

BOOL CALLBACK GetRunningWindows(HWND window, LPARAM data) {
  std::vector<std::string>& result =
      *reinterpret_cast<std::vector<std::string>*>(data);
  std::string process = ProcessName(window);

  // some processes have windows but end in ".tmp" and should be ignored
  std::string tmp = ".tmp";
  if (process.length() < tmp.length() ||
      process.compare(process.length() - tmp.length(), tmp.length(), tmp) !=
          0) {
    result.push_back(process);
  }

  return TRUE;
}

void MouseDown(const std::string& button) {
  if (button == "right") {
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
  } else if (button == "middle") {
    mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
  } else {
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
  }
}

void MouseUp(const std::string& button) {
  if (button == "right") {
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
  } else if (button == "middle") {
    mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
  } else {
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
  }
}

void PressKey(const std::string& key, std::vector<std::string> modifiers) {
  for (std::string modifier : modifiers) {
    ToggleKey(modifier, true);
  }

  ToggleKey(key, true);
  ToggleKey(key, false);

  for (std::string modifier : modifiers) {
    ToggleKey(modifier, false);
  }
}

std::string ProcessName(HWND window) {
  DWORD pid = 0;
  GetWindowThreadProcessId(window, &pid);

  wchar_t path[1024];
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  GetProcessImageFileNameW(process, path, 1024);
  std::wstring wide(path);
  return std::string(wide.begin(), wide.end());
}

void SetMouseLocation(int x, int y) { SetCursorPos(x, y); }

void ToggleKey(const std::string& key, bool down) {
  // first, look for a hard-coded virtual key (e.g., for non-alphanumeric
  // characters)
  bool shift = false;
  bool altgr = false;
  int virtualKey = VirtualKey(key);

  // if we didn't find one, then convert the key's character into a
  // keyboard-indepdent virutal key
  if (virtualKey == -1) {
    // this function returns the key in the low-order byte and whether or not
    // shift is required in the high-order byte
    int k = VkKeyScanA(key[0]);
    virtualKey = k & 0xff;
    shift = (((k >> 8) & 0xff) & 1) == 1;
    altgr = (((k >> 8) & 0xff) & 4) == 1;
  }

  if (shift) {
    ToggleKey("shift", true);
  }
  if (altgr) {
    ToggleKey("alt", true);
    ToggleKey("control", true);
  }

  INPUT event;
  event.type = INPUT_KEYBOARD;
  event.ki.wVk = virtualKey;
  event.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
  SendInput(1, &event, sizeof(INPUT));

  if (shift) {
    ToggleKey("shift", false);
  }
  if (altgr) {
    ToggleKey("control", false);
    ToggleKey("alt", false);
  }
}

int VirtualKey(const std::string& key) {
  if (key == "left") {
    return VK_LEFT;
  } else if (key == "right") {
    return VK_RIGHT;
  } else if (key == "up") {
    return VK_UP;
  } else if (key == "down") {
    return VK_DOWN;
  } else if (key == "control" || key == "ctrl") {
    return VK_CONTROL;
  } else if (key == "alt") {
    return VK_MENU;
  } else if (key == "shift") {
    return VK_SHIFT;
  } else if (key == "backspace") {
    return VK_BACK;
  } else if (key == "delete") {
    return VK_DELETE;
  } else if (key == "tab" || key == "\t") {
    return VK_TAB;
  } else if (key == "space" || key == " ") {
    return VK_SPACE;
  } else if (key == "caps") {
    return VK_CAPITAL;
  } else if (key == "meta" || key == "win" || key == "windows") {
    return VK_LWIN;
  } else if (key == "escape") {
    return VK_ESCAPE;
  } else if (key == "enter" || key == "return" || key == "\n") {
    return VK_RETURN;
  } else if (key == "pageup") {
    return VK_PRIOR;
  } else if (key == "pagedown") {
    return VK_NEXT;
  } else if (key == "home") {
    return VK_HOME;
  } else if (key == "end") {
    return VK_END;
  } else if (key == "f1") {
    return VK_F1;
  } else if (key == "f2") {
    return VK_F2;
  } else if (key == "f3") {
    return VK_F3;
  } else if (key == "f4") {
    return VK_F4;
  } else if (key == "f5") {
    return VK_F5;
  } else if (key == "f6") {
    return VK_F6;
  } else if (key == "f7") {
    return VK_F7;
  } else if (key == "f8") {
    return VK_F8;
  } else if (key == "f9") {
    return VK_F9;
  } else if (key == "f10") {
    return VK_F10;
  } else if (key == "f11") {
    return VK_F11;
  } else if (key == "f12") {
    return VK_F12;
  }

  return -1;
}
}  // namespace driver
