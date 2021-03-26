#include <windows.h>

// must be included after <windows.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <winuser.h>

#include <algorithm>
#include <iostream>
#include <string>
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

std::string GetClipboard() {
  if (!OpenClipboard(NULL)) {
    return "";
  }

  HANDLE data = GetClipboardData(CF_TEXT);
  if (data == NULL) {
    return "";
  }

  char* text = static_cast<char*>(GlobalLock(data));
  if (text == NULL) {
    return "";
  }

  std::string result(text);
  GlobalUnlock(data);
  CloseClipboard();
  return result;
}

std::tuple<std::string, int> GetEditorState(bool fallback) {
  std::tuple<std::string, int> result;
  std::get<0>(result) = "";
  std::get<1>(result) = 0;
  if (!fallback) {
    return result;
  }

  std::string previous = GetClipboard();
  PressKey("home", std::vector<std::string>{"control", "shift"});
  PressKey("c", std::vector<std::string>{"control"});
  Sleep(10);
  PressKey("right", std::vector<std::string>{});
  std::string left = GetClipboard();

  PressKey("end", std::vector<std::string>{"control", "shift"});
  PressKey("c", std::vector<std::string>{"control"});
  Sleep(10);
  PressKey("left", std::vector<std::string>{});
  std::string right = GetClipboard();

  if (previous != "") {
    const size_t n = previous.length() + 1;
    HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE, n);
    memcpy(GlobalLock(data), previous.c_str(), n);
    GlobalUnlock(data);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, data);
  }

  std::get<0>(result) = left + right;
  std::get<1>(result) = left.length();
  return result;
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

std::tuple<int, bool, bool, int> GetVirtualKeyAndModifiers(
    const std::string& key) {
  std::tuple<int, bool, bool, int> result;
  std::get<0>(result) = -1;

  if (key == "left") {
    std::get<0>(result) = VK_LEFT;
  } else if (key == "right") {
    std::get<0>(result) = VK_RIGHT;
  } else if (key == "up") {
    std::get<0>(result) = VK_UP;
  } else if (key == "down") {
    std::get<0>(result) = VK_DOWN;
  } else if (key == "control" || key == "ctrl" || key == "commandOrControl") {
    std::get<0>(result) = VK_CONTROL;
  } else if (key == "alt" || key == "option") {
    std::get<0>(result) = VK_MENU;
  } else if (key == "shift") {
    std::get<0>(result) = VK_SHIFT;
  } else if (key == "backspace") {
    std::get<0>(result) = VK_BACK;
  } else if (key == "delete") {
    std::get<0>(result) = VK_DELETE;
  } else if (key == "tab" || key == "\t") {
    std::get<0>(result) = VK_TAB;
  } else if (key == "space" || key == " ") {
    std::get<0>(result) = VK_SPACE;
  } else if (key == "caps") {
    std::get<0>(result) = VK_CAPITAL;
  } else if (key == "meta" || key == "win" || key == "windows") {
    std::get<0>(result) = VK_LWIN;
  } else if (key == "escape") {
    std::get<0>(result) = VK_ESCAPE;
  } else if (key == "enter" || key == "return" || key == "\n") {
    std::get<0>(result) = VK_RETURN;
  } else if (key == "pageup") {
    std::get<0>(result) = VK_PRIOR;
  } else if (key == "pagedown") {
    std::get<0>(result) = VK_NEXT;
  } else if (key == "home") {
    std::get<0>(result) = VK_HOME;
    std::get<3>(result) = 1;
  } else if (key == "end") {
    std::get<0>(result) = VK_END;
    std::get<3>(result) = 1;
  } else if (key == "f1") {
    std::get<0>(result) = VK_F1;
  } else if (key == "f2") {
    std::get<0>(result) = VK_F2;
  } else if (key == "f3") {
    std::get<0>(result) = VK_F3;
  } else if (key == "f4") {
    std::get<0>(result) = VK_F4;
  } else if (key == "f5") {
    std::get<0>(result) = VK_F5;
  } else if (key == "f6") {
    std::get<0>(result) = VK_F6;
  } else if (key == "f7") {
    std::get<0>(result) = VK_F7;
  } else if (key == "f8") {
    std::get<0>(result) = VK_F8;
  } else if (key == "f9") {
    std::get<0>(result) = VK_F9;
  } else if (key == "f10") {
    std::get<0>(result) = VK_F10;
  } else if (key == "f11") {
    std::get<0>(result) = VK_F11;
  } else if (key == "f12") {
    std::get<0>(result) = VK_F12;
  }

  // convert the key's character into a keyboard-indepdent virtual key
  if (std::get<0>(result) == -1) {
    // this function returns the key in the low-order byte and whether or not
    // shift & altgr are required in the high-order byte
    int k = VkKeyScanA(key[0]);
    std::get<0>(result) = k & 0xff;
    std::get<1>(result) = ((k >> 8) & 1) == 1;
    std::get<2>(result) = (k >> 9) == 3;
  }

  return result;
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

  Sleep(2);
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
  std::tuple<int, bool, bool, int> keycode = GetVirtualKeyAndModifiers(key);
  bool shift = std::get<1>(keycode);
  bool altgr = std::get<2>(keycode);

  if (shift) {
    ToggleKey("shift", true);
  }
  if (altgr) {
    ToggleKey("alt", true);
    ToggleKey("control", true);
  }

  INPUT event;
  event.type = INPUT_KEYBOARD;
  event.ki.wVk = std::get<0>(keycode);
  event.ki.dwFlags = (down ? 0 : KEYEVENTF_KEYUP) | std::get<3>(keycode);
  SendInput(1, &event, sizeof(INPUT));

  if (shift) {
    ToggleKey("shift", false);
  }
  if (altgr) {
    ToggleKey("control", false);
    ToggleKey("alt", false);
  }
}

}  // namespace driver
