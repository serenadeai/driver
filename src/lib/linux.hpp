#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>

namespace driver {

void GetKeycode(Display* display, const std::string& key, int* keycode,
                bool* shift) {
  // convert our key names to the corresponding x11 key
  std::string mapped = key;
  if (key == "Escape") {
    mapped = "escape";
  } else if (key == "control" || key == "ctrl") {
    mapped = "Control_L";
  } else if (key == "alt") {
    mapped = "Alt_L";
  } else if (key == "meta" || key == "windows" || key == "win") {
    mapped = "Super_L";
  } else if (key == "shift") {
    mapped = "Shift_L";
  } else if (key == "`") {
    mapped = "grave";
  } else if (key == "~") {
    mapped = "asciitilde";
  } else if (key == "!") {
    mapped = "exclam";
  } else if (key == "@") {
    mapped = "at";
  } else if (key == "#") {
    mapped = "numbersign";
  } else if (key == "$") {
    mapped = "dollar";
  } else if (key == "%") {
    mapped = "percent";
  } else if (key == "^") {
    mapped = "asciicircum";
  } else if (key == "&") {
    mapped = "ampersand";
  } else if (key == "*") {
    mapped = "asterisk";
  } else if (key == "(") {
    mapped = "parenleft";
  } else if (key == ")") {
    mapped = "parenright";
  } else if (key == "-") {
    mapped = "minus";
  } else if (key == "_") {
    mapped = "underscore";
  } else if (key == "+") {
    mapped = "plus";
  } else if (key == "=") {
    mapped = "equal";
  } else if (key == "backspace") {
    mapped = "BackSpace";
  } else if (key == "tab") {
    mapped = "Tab";
  } else if (key == "[") {
    mapped = "bracketleft";
  } else if (key == "{") {
    mapped = "braceleft";
  } else if (key == "]") {
    mapped = "bracketright";
  } else if (key == "}") {
    mapped = "braceright";
  } else if (key == "\\") {
    mapped = "backslash";
  } else if (key == "|") {
    mapped = "bar";
  } else if (key == "caps") {
    mapped = "Caps_Lock";
  } else if (key == ";") {
    mapped = "semicolon";
  } else if (key == ":") {
    mapped = "colon";
  } else if (key == "'") {
    mapped = "apostrophe";
  } else if (key == "\"") {
    mapped = "quotedbl";
  } else if (key == "enter" || key == "return") {
    mapped = "Return";
  } else if (key == ",") {
    mapped = "comma";
  } else if (key == "<") {
    mapped = "less";
  } else if (key == ".") {
    mapped = "period";
  } else if (key == ">") {
    mapped = "greater";
  } else if (key == "/") {
    mapped = "slash";
  } else if (key == "?") {
    mapped = "question";
  } else if (key == " ") {
    mapped = "space";
  } else if (key == "home") {
    mapped = "Home";
  } else if (key == "end") {
    mapped = "End";
  } else if (key == "left") {
    mapped = "Left";
  } else if (key == "right") {
    mapped = "Right";
  } else if (key == "up") {
    mapped = "Up";
  } else if (key == "down") {
    mapped = "Down";
  } else if (key == "pageup") {
    mapped = "Prior";
  } else if (key == "pagedown") {
    mapped = "Next";
  } else if (key == "delete") {
    mapped = "Delete";
  } else if (key == "f1") {
    mapped = "F1";
  } else if (key == "f2") {
    mapped = "F2";
  } else if (key == "f3") {
    mapped = "F3";
  } else if (key == "f4") {
    mapped = "F4";
  } else if (key == "f5") {
    mapped = "F5";
  } else if (key == "f6") {
    mapped = "F6";
  } else if (key == "f7") {
    mapped = "F7";
  } else if (key == "f8") {
    mapped = "F8";
  } else if (key == "f9") {
    mapped = "F9";
  } else if (key == "f10") {
    mapped = "F10";
  } else if (key == "f11") {
    mapped = "F11";
  } else if (key == "f12") {
    mapped = "F12";
  }

  // search the printable ASCII table range for the string that would be typed
  // with the given keysym and shift modifier combination. because keycodes
  // refer to physical keys, we need to handle the state of the shift key
  // ourselves, and we also need to make sure we're using the current keyboard
  // layout, not just assuming en_US
  XkbStateRec state;
  XkbGetState(display, XkbUseCoreKbd, &state);
  for (int i = 0; i < 256; i++) {
    for (int modifier = 0; modifier < 2; modifier++) {
      KeySym k;
      XkbLookupKeySym(display, i, (state.group << 13) | modifier, NULL, &k);
      const char* s = XKeysymToString(k);
      if (s == NULL) {
        continue;
      }

      std::string name(s);
      if (mapped == name) {
        *keycode = i;
        *shift = modifier == 1;
        return;
      }
    }
  }
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
  int keycode = -1;
  bool shift = false;
  GetKeycode(display, key, &keycode, &shift);
  if (keycode == -1) {
    return;
  }

  XTestFakeKeyEvent(display, keycode, down, CurrentTime);
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
  int keycode = -1;
  bool shift = false;
  GetKeycode(display, key, &keycode, &shift);
  if (keycode == -1) {
    return;
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
