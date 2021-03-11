#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string>
#include <tuple>
#include <vector>

namespace driver {

void Click(Display* display, const std::string& button, int count);
void FocusApplication(const std::string& application);
std::string GetActiveApplication();
std::vector<Window> GetAllWindows(Display* display);
std::string GetClipboard(Display* display, Window window);
std::tuple<std::string, int> GetEditorState(Display* display, bool fallback);
std::tuple<int, bool, bool> GetKeycodeAndModifiers(Display* display,
                                                   const std::string& key);
std::tuple<int, int> GetMouseLocation();
void GetProperty(Display* display, Window window, const std::string& property,
                 unsigned char** result, unsigned long* length);
std::vector<std::string> GetRunningApplications();
void MouseDown(Display* display, const std::string& button);
void MouseUp(Display* display, const std::string& button);
void PressKey(Display* display, std::string key,
              std::vector<std::string> modifiers);
std::string ProcessName(Display* display, Window window);
void SetMouseLocation(int x, int y);
void ToggleKey(Display* display, const std::string& key, bool down);
void ToLower(std::string& s);

}  // namespace driver
