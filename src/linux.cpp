#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <climits>
#include <fstream>
#include <streambuf>
#include <string>
#include <tuple>
#include <vector>

#include "linux.hpp"

namespace driver
{

  void Click(Display *display, const std::string &button, int count)
  {
    for (int i = 0; i < count; i++)
    {
      MouseDown(display, button);
      MouseUp(display, button);
    }
  }

  void FocusApplication(const std::string &application)
  {
    Display *display = XOpenDisplay(NULL);
    std::string lower = application;
    ToLower(lower);

    std::vector<Window> windows = GetAllWindows(display);
    for (Window window : windows)
    {
      std::string name = ProcessName(display, window);
      if (name.find(application) != std::string::npos)
      {
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
                   (XEvent *)&event);
        break;
      }
    }

    XCloseDisplay(display);
  }

  std::string GetActiveApplication()
  {
    Display *display = XOpenDisplay(NULL);
    Window root = XDefaultRootWindow(display);
    unsigned long length = 0;
    unsigned char *property = 0;
    GetProperty(display, root, "_NET_ACTIVE_WINDOW", &property, &length);

    Window *window = (Window *)property;
    std::string result = ProcessName(display, *window);

    XFree(window);
    XCloseDisplay(display);
    return result;
  }

  std::vector<Window> GetAllWindows(Display *display)
  {
    Window root = XDefaultRootWindow(display);
    unsigned long length = 0;
    unsigned char *property = 0;
    GetProperty(display, root, "_NET_CLIENT_LIST", &property, &length);

    std::vector<Window> result;
    Window *windows = (Window *)property;
    for (unsigned long i = 0; i < length; i++)
    {
      result.push_back(windows[i]);
    }

    XFree(windows);
    return result;
  }

  std::string GetClipboard(Display *display, Window window)
  {
    Atom buffer = XInternAtom(display, "CLIPBOARD", False);
    Atom format = XInternAtom(display, "STRING", False);
    Atom property = XInternAtom(display, "XSEL_DATA", False);
    Atom incr = XInternAtom(display, "INCR", False);

    std::string result = "";
    char *data = NULL;
    int dataBits = 0;
    unsigned long dataSize = 0;
    unsigned long dataTail = 0;
    XEvent event;
    XConvertSelection(display, buffer, format, property, window, CurrentTime);
    do
    {
      XNextEvent(display, &event);
    } while (event.type != SelectionNotify ||
             event.xselection.selection != buffer);

    if (event.xselection.property)
    {
      XGetWindowProperty(display, window, property, 0, LONG_MAX / 4, False,
                         AnyPropertyType, &format, &dataBits, &dataSize,
                         &dataTail, (unsigned char **)&data);

      if (data != NULL && format != incr)
      {
        result = std::string(data);
      }

      if (data != NULL)
      {
        XFree(data);
      }
    }

    return result;
  }

  std::tuple<std::string, int, bool> GetEditorState(Display *display, bool fallback)
  {
    std::tuple<std::string, int, bool> result;
    if (!fallback)
    {
      return result;
    }

    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0,
                                        1, 1, 0, color, color);
    PressKey(display, "home", std::vector<std::string>{"control", "shift"});
    PressKey(display, "c", std::vector<std::string>{"control"});
    usleep(10000);
    PressKey(display, "right", std::vector<std::string>{});
    std::string left = GetClipboard(display, window);

    PressKey(display, "end", std::vector<std::string>{"control", "shift"});
    PressKey(display, "c", std::vector<std::string>{"control"});
    usleep(10000);
    PressKey(display, "left", std::vector<std::string>{});
    std::string right = GetClipboard(display, window);

    std::get<0>(result) = left + right;
    std::get<1>(result) = left.length();
    std::get<2>(result) = true;
    return result;
  }

  std::tuple<int, bool, bool> GetKeycodeAndModifiers(Display *display,
                                                     const std::string &key)
  {
    std::tuple<int, bool, bool> result;
    std::get<0>(result) = -1;

    // convert our key names to the corresponding x11 key
    std::string mapped = key;
    if (key == "escape")
    {
      mapped = "Escape";
    }
    else if (key == "control" || key == "ctrl" || key == "commandOrControl")
    {
      mapped = "Control_L";
    }
    else if (key == "alt" || key == "option")
    {
      mapped = "Alt_L";
    }
    else if (key == "altgr")
    {
      mapped = "ISO_Level3_Shift";
    }
    else if (key == "meta" || key == "windows" || key == "win")
    {
      mapped = "Super_L";
    }
    else if (key == "shift")
    {
      mapped = "Shift_L";
    }
    else if (key == "`")
    {
      mapped = "grave";
    }
    else if (key == "~")
    {
      mapped = "asciitilde";
    }
    else if (key == "!")
    {
      mapped = "exclam";
    }
    else if (key == "@")
    {
      mapped = "at";
    }
    else if (key == "#")
    {
      mapped = "numbersign";
    }
    else if (key == "$")
    {
      mapped = "dollar";
    }
    else if (key == "%")
    {
      mapped = "percent";
    }
    else if (key == "^")
    {
      mapped = "asciicircum";
    }
    else if (key == "&")
    {
      mapped = "ampersand";
    }
    else if (key == "*")
    {
      mapped = "asterisk";
    }
    else if (key == "(")
    {
      mapped = "parenleft";
    }
    else if (key == ")")
    {
      mapped = "parenright";
    }
    else if (key == "-")
    {
      mapped = "minus";
    }
    else if (key == "_")
    {
      mapped = "underscore";
    }
    else if (key == "+")
    {
      mapped = "plus";
    }
    else if (key == "=")
    {
      mapped = "equal";
    }
    else if (key == "backspace")
    {
      mapped = "BackSpace";
    }
    else if (key == "tab" || key == "\t")
    {
      mapped = "Tab";
    }
    else if (key == "[")
    {
      mapped = "bracketleft";
    }
    else if (key == "{")
    {
      mapped = "braceleft";
    }
    else if (key == "]")
    {
      mapped = "bracketright";
    }
    else if (key == "}")
    {
      mapped = "braceright";
    }
    else if (key == "\\")
    {
      mapped = "backslash";
    }
    else if (key == "|")
    {
      mapped = "bar";
    }
    else if (key == "caps")
    {
      mapped = "Caps_Lock";
    }
    else if (key == ";")
    {
      mapped = "semicolon";
    }
    else if (key == ":")
    {
      mapped = "colon";
    }
    else if (key == "'")
    {
      mapped = "apostrophe";
    }
    else if (key == "\"")
    {
      mapped = "quotedbl";
    }
    else if (key == "enter" || key == "return" || key == "\n")
    {
      mapped = "Return";
    }
    else if (key == ",")
    {
      mapped = "comma";
    }
    else if (key == "<")
    {
      mapped = "less";
    }
    else if (key == ".")
    {
      mapped = "period";
    }
    else if (key == ">")
    {
      mapped = "greater";
    }
    else if (key == "/")
    {
      mapped = "slash";
    }
    else if (key == "?")
    {
      mapped = "question";
    }
    else if (key == "space" || key == " ")
    {
      mapped = "space";
    }
    else if (key == "home")
    {
      mapped = "Home";
    }
    else if (key == "end")
    {
      mapped = "End";
    }
    else if (key == "left")
    {
      mapped = "Left";
    }
    else if (key == "right")
    {
      mapped = "Right";
    }
    else if (key == "up")
    {
      mapped = "Up";
    }
    else if (key == "down")
    {
      mapped = "Down";
    }
    else if (key == "pageup")
    {
      mapped = "Prior";
    }
    else if (key == "pagedown")
    {
      mapped = "Next";
    }
    else if (key == "delete")
    {
      mapped = "Delete";
    }
    else if (key == "f1")
    {
      mapped = "F1";
    }
    else if (key == "f2")
    {
      mapped = "F2";
    }
    else if (key == "f3")
    {
      mapped = "F3";
    }
    else if (key == "f4")
    {
      mapped = "F4";
    }
    else if (key == "f5")
    {
      mapped = "F5";
    }
    else if (key == "f6")
    {
      mapped = "F6";
    }
    else if (key == "f7")
    {
      mapped = "F7";
    }
    else if (key == "f8")
    {
      mapped = "F8";
    }
    else if (key == "f9")
    {
      mapped = "F9";
    }
    else if (key == "f10")
    {
      mapped = "F10";
    }
    else if (key == "f11")
    {
      mapped = "F11";
    }
    else if (key == "f12")
    {
      mapped = "F12";
    }

    // search the printable ASCII table range for the string that would be typed
    // with the given keysym and shift modifier combination. because keycodes
    // refer to physical keys, we need to handle the state of the shift key
    // ourselves, and we also need to make sure we're using the current keyboard
    // layout, not just assuming en_US
    XkbStateRec state;
    XkbGetState(display, XkbUseCoreKbd, &state);
    for (int i = 0; i < 256; i++)
    {
      for (int shift = 0; shift < 2; shift++)
      {
        for (int altgr = 0; altgr < 2; altgr++)
        {
          int modifier = state.group << 13;
          if (shift == 1)
          {
            modifier |= ShiftMask;
          }
          if (altgr == 1)
          {
            modifier |= Mod5Mask;
          }

          KeySym k;
          XkbLookupKeySym(display, i, modifier, NULL, &k);
          const char *s = XKeysymToString(k);
          if (s == NULL)
          {
            continue;
          }

          std::string name(s);
          if (mapped == name)
          {
            std::get<0>(result) = i;
            std::get<1>(result) = shift == 1;
            std::get<2>(result) = altgr == 1;
            return result;
          }
        }
      }
    }

    return result;
  }

  int GetMouseButton(const std::string &button)
  {
    if (button == "middle")
    {
      return Button2;
    }
    else if (button == "right")
    {
      return Button3;
    }

    return Button1;
  }

  std::tuple<int, int> GetMouseLocation()
  {
    std::tuple<int, int> result;
    Display *display = XOpenDisplay(NULL);
    Window root = XDefaultRootWindow(display);
    Window rootReturn;
    Window childReturn;
    int x = 0;
    int y = 0;
    int windowX = 0;
    int windowY = 0;
    unsigned int mask;
    XQueryPointer(display, root, &rootReturn, &childReturn, &x, &y, &windowX,
                  &windowY, &mask);

    std::get<0>(result) = x;
    std::get<1>(result) = y;
    XCloseDisplay(display);
    return result;
  }

  void GetProperty(Display *display, Window window, const std::string &property,
                   unsigned char **result, unsigned long *length)
  {
    Atom atom = XInternAtom(display, property.c_str(), 1);
    unsigned long actual_type = 0;
    int actual_format = 0;
    unsigned long bytes_after = 0;
    XGetWindowProperty(display, window, atom, 0, 1024, 0, 0, &actual_type,
                       &actual_format, length, &bytes_after, result);
  }

  std::vector<std::string> GetRunningApplications()
  {
    Display *display = XOpenDisplay(NULL);
    std::vector<Window> windows = GetAllWindows(display);

    std::vector<std::string> result;
    for (Window window : windows)
    {
      std::string name = ProcessName(display, window);
      name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
      result.push_back(name);
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
  }

  void MouseDown(Display *display, const std::string &button)
  {
    XTestFakeButtonEvent(display, GetMouseButton(button), true, 0);
    XFlush(display);
    usleep(10000);
  }

  void MouseUp(Display *display, const std::string &button)
  {
    XTestFakeButtonEvent(display, GetMouseButton(button), false, 0);
    XFlush(display);
    usleep(10000);
  }

  void PressKey(Display *display, std::string key,
                std::vector<std::string> modifiers)
  {
    std::tuple<int, bool, bool> keycodeAndModifiers =
        GetKeycodeAndModifiers(display, key);
    int keycode = std::get<0>(keycodeAndModifiers);
    bool shift = std::get<1>(keycodeAndModifiers);
    bool altgr = std::get<2>(keycodeAndModifiers);

    if (keycode == -1)
    {
      return;
    }

    if (shift)
    {
      ToggleKey(display, "shift", true);
    }
    if (altgr)
    {
      ToggleKey(display, "altgr", true);
    }

    for (std::string modifier : modifiers)
    {
      ToggleKey(display, modifier, true);
    }

    ToggleKey(display, key, true);
    ToggleKey(display, key, false);

    for (std::string modifier : modifiers)
    {
      ToggleKey(display, modifier, false);
    }

    if (altgr)
    {
      ToggleKey(display, "altgr", false);
    }
    if (shift)
    {
      ToggleKey(display, "shift", false);
    }

    usleep(3000);
  }

  std::string ProcessName(Display *display, Window window)
  {
    unsigned long length = 0;
    unsigned char *property = 0;
    GetProperty(display, window, "_NET_WM_PID", &property, &length);

    unsigned long *pid = (unsigned long *)property;
    std::ifstream t(std::string("/proc/") + std::to_string(*pid) +
                    std::string("/cmdline"));
    std::string path((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    XFree(pid);
    ToLower(path);
    return path;
  }

  void SetMouseLocation(int x, int y)
  {
    Display *display = XOpenDisplay(NULL);
    XWarpPointer(display, None, XDefaultRootWindow(display), 0, 0, 0, 0, x, y);
    XCloseDisplay(display);
  }

  void ToggleKey(Display *display, const std::string &key, bool down)
  {
    std::tuple<int, bool, bool> keycodeAndModifiers =
        GetKeycodeAndModifiers(display, key);
    int keycode = std::get<0>(keycodeAndModifiers);
    if (keycode == -1)
    {
      return;
    }

    XTestFakeKeyEvent(display, keycode, down, CurrentTime);
    XFlush(display);
  }

  void ToLower(std::string &s)
  {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }

} // namespace driver
