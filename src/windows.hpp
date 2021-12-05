#include <windows.h>
#include <winuser.h>

#include <tuple>
#include <vector>

namespace driver {

void Click(const std::string& button, int count);
void FocusApplication(const std::string& application);
BOOL CALLBACK FocusWindow(HWND window, LPARAM data);
std::string GetActiveApplication();
std::tuple<int, int, int, int> GetActiveApplicationWindowBounds();
std::string GetClipboard();
std::tuple<std::string, int, bool> GetEditorState();
std::tuple<int, int> GetMouseLocation();
std::vector<std::string> GetRunningApplications();
BOOL CALLBACK GetRunningWindows(HWND window, LPARAM data);
std::tuple<int, bool, bool, int> GetVirtualKeyAndModifiers(
    const std::string& key);
void InitializeUIAutomation();
void MouseDown(const std::string& button);
void MouseUp(const std::string& button);
void PressKey(const std::string& key, std::vector<std::string> modifiers,
              int delay);
std::string ProcessName(HWND window);
void RemoveNonASCII(std::string& s);
void SetMouseLocation(int x, int y);
void ToggleKey(const std::string& key, bool down);

}  // namespace driver
