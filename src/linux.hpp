#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string>
#include <vector>

namespace driver {

void Click(const std::string& buttonType, int count);
void FocusApplication(const std::string& application);
std::string GetActiveApplication();
void GetKeycode(Display* display, const std::string& key, int* keycode,
                bool* shift);
void GetProperty(Display* display, Window window, const std::string& property,
                 unsigned char** result, unsigned long* length);
std::vector<std::string> GetRunningApplications();
void PressKey(Display* display, std::string key,
              std::vector<std::string> modifiers);
std::string ProcessName(Display* display, Window window);
void SetMouseLocation(int x, int y);
void ToggleKey(Display* display, const std::string& key, bool down);
void ToLower(std::string& s);

}  // namespace driver
