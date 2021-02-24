#include <windef.h>
#include <winuser.h>

#include <vector>

namespace driver {

void Click(const std::string& buttonType, int count);
void FocusApplication(const std::string& application);
BOOL CALLBACK FocusWindow(HWND window, LPARAM data);
std::string GetActiveApplication();
std::vector<std::string> GetRunningApplications();
BOOL CALLBACK GetRunningWindows(HWND window, LPARAM data);
void PressKey(const std::string& key, std::vector<std::string> modifiers);
std::string ProcessName(HWND window);
void SetMouseLocation(int x, int y);
void ToggleKey(const std::string& key, bool down);
int VirtualKey(const std::string& key);

}  // namespace driver
