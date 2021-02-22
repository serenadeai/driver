#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>

namespace driver {

unsigned long GetKeySym(Display* display, const std::string& key) {
  if (key == "left") {
    return XK_Left;
  } else if (key == "right") {
    return XK_Right;
  } else if (key == "up") {
    return XK_Up;
  } else if (key == "down") {
    return XK_Down;
  } else if (key == "control" || key == "ctrl") {
    return XK_Control_L;
  } else if (key == "alt") {
    return XK_Alt_L;
  } else if (key == "end") {
    return XK_End;
  } else if (key == "pageup") {
    return XK_Page_Up;
  } else if (key == "pagedown") {
    return XK_Page_Down;
  } else if (key == "enter" || key == "return") {
    return XK_Return;
  } else if (key == "delete") {
    return XK_Delete;
  } else if (key == "home") {
    return XK_Home;
  } else if (key == "escape") {
    return XK_Escape;
  } else if (key == "backspace") {
    return XK_BackSpace;
  } else if (key == "meta" || key == "windows" || key == "win") {
    return XK_Meta_L;
  } else if (key == "caps") {
    return XK_Caps_Lock;
  } else if (key == "shift") {
    return XK_Shift_L;
  } else if (key == "tab") {
    return XK_Tab;
  } else if (key == "space") {
    return XK_space;
  } else if (key == "f1") {
    return XK_F1;
  } else if (key == "f2") {
    return XK_F2;
  } else if (key == "f3") {
    return XK_F3;
  } else if (key == "f4") {
    return XK_F4;
  } else if (key == "f5") {
    return XK_F5;
  } else if (key == "f6") {
    return XK_F6;
  } else if (key == "f7") {
    return XK_F7;
  } else if (key == "f8") {
    return XK_F8;
  } else if (key == "f9") {
    return XK_F9;
  } else if (key == "f10") {
    return XK_F10;
  } else if (key == "f11") {
    return XK_F11;
  } else if (key == "f12") {
    return XK_F12;
  }

  return (KeySym)key[0];
}

void ToLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

void GetProperty(Display* display, Window window, const std::string& property,
                 unsigned char** result, unsigned long* length) {
  Atom atom = XInternAtom(display, property.c_str(), 1);
  unsigned long actual_type = 0;
  int actual_format = 0;
  unsigned long bytes_after = 0;
  XGetWindowProperty(display, window, atom, 0, 1024, 0, 0, &actual_type,
                     &actual_format, length, &bytes_after, result);
}

std::string ProcessName(Display* display, Window window) {
  unsigned long length = 0;
  unsigned char* property = 0;
  GetProperty(display, window, "_NET_WM_PID", &property, &length);

  unsigned long* pid = (unsigned long*)property;
  std::ifstream t(std::string("/proc/") + std::to_string(*pid) +
                  std::string("/cmdline"));
  std::string path((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  XFree(pid);
  ToLower(path);
  return path;
}

void ToggleKey(Display* display, const std::string& key, bool down) {
  XTestFakeKeyEvent(display, XKeysymToKeycode(display, GetKeySym(display, key)),
                    down, CurrentTime);
  XFlush(display);
}

void Click(const std::string& buttonType, int count) {
  Display* display = XOpenDisplay(NULL);
  int button = Button1;
  if (buttonType == "middle") {
    button = Button2;
  } else if (buttonType == "right") {
    button = Button3;
  }

  for (int i = 0; i < count; i++) {
    XTestFakeButtonEvent(display, button, true, 0);
    XFlush(display);
    usleep(10000);
    XTestFakeButtonEvent(display, button, false, 0);
    XFlush(display);
  }

  XCloseDisplay(display);
}

void FocusApplication(const std::string& application) {
  std::string lower = application;
  ToLower(lower);

  Display* display = XOpenDisplay(NULL);
  Window root = XDefaultRootWindow(display);

  unsigned long length = 0;
  unsigned char* property = 0;
  GetProperty(display, root, "_NET_CLIENT_LIST", &property, &length);

  Window* windows = (Window*)property;
  for (unsigned long i = 0; i < length; i++) {
    Window window = windows[i];
    std::string name = ProcessName(display, window);
    if (name.find(application) != std::string::npos) {
      XClientMessageEvent event;
      event.type = ClientMessage;
      event.display = display;
      event.window = window;
      event.message_type =
          XInternAtom(display, std::string("_NET_ACTIVE_WINDOW").c_str(), 1);
      event.format = 32;
      event.data.l[0] = 1;
      event.data.l[1] = CurrentTime;

      Window root = XDefaultRootWindow(display);
      XSendEvent(display, root, 0,
                 SubstructureRedirectMask | SubstructureNotifyMask,
                 (XEvent*)&event);
    }
  }

  XFree(windows);
  XCloseDisplay(display);
}

std::string GetActiveApplication() {
  Display* display = XOpenDisplay(NULL);
  Window root = XDefaultRootWindow(display);
  unsigned long length = 0;
  unsigned char* property = 0;
  GetProperty(display, root, "_NET_ACTIVE_WINDOW", &property, &length);

  Window* window = (Window*)property;
  std::string result = ProcessName(display, *window);

  XFree(window);
  XCloseDisplay(display);
  return result;
}

std::vector<std::string> GetRunningApplications() {
  Display* display = XOpenDisplay(NULL);
  Window root = XDefaultRootWindow(display);

  unsigned long length = 0;
  unsigned char* property = 0;
  GetProperty(display, root, "_NET_CLIENT_LIST", &property, &length);

  Window* windows = (Window*)property;
  std::vector<std::string> result;
  for (unsigned long i = 0; i < length; i++) {
    Window window = windows[i];
    std::string name = ProcessName(display, window);
    name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
    result.push_back(name);
  }

  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());

  XFree(windows);
  return result;
}

void PressKey(Display* display, std::string key,
              std::vector<std::string> modifiers) {
  bool shift = false;
  unsigned long keysym = GetKeySym(display, key);

  // keycodes only refer to physical keys, so we need to determine whether or
  // not we need to hit the shift key manually. to do so, we see what character
  // would be typed if we use the same keycode with the shift key also pressed,
  // and if that matches the desired key, then we know we need to press shift
  XKeyPressedEvent event;
  event.type = KeyPress;
  event.display = display;
  event.keycode = XKeysymToKeycode(display, keysym);
  event.state = ShiftMask;

  char buffer[32];
  KeySym ignore;
  XLookupString(&event, buffer, 32, &ignore, 0);
  if (key == buffer) {
    shift = true;
  }

  if (shift) {
    ToggleKey(display, "shift", true);
  }

  for (std::string modifier : modifiers) {
    ToggleKey(display, modifier, true);
  }

  ToggleKey(display, key, true);
  ToggleKey(display, key, false);

  for (std::string modifier : modifiers) {
    ToggleKey(display, modifier, false);
  }

  if (shift) {
    ToggleKey(display, "shift", false);
  }
}

void SetMouseLocation(int x, int y) {
  Display* display = XOpenDisplay(NULL);
  XWarpPointer(display, None, XDefaultRootWindow(display), 0, 0, 0, 0, x, y);
  XCloseDisplay(display);
}

}  // namespace driver
