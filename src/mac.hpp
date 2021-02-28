#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>

#include <string>
#include <vector>

#define kVirtualKeyNotFound 65535
#define kAXOpenAction CFSTR("AXOpen")
#define kAXEnhancedUserInterfaceAttribute CFSTR("AXEnhancedUserInterface")
#define kAXManualAccessibilityAttribute CFSTR("AXManualAccessibility")

namespace driver {

void Click(const std::string& button, int count);
bool ClickButton(AXUIElementRef element, const std::string& button);
void ClickButton(const std::string& button);
AXUIElementRef CreateActiveTextFieldRef();
AXUIElementRef CreateActiveWindowRef();
CFArrayRef CreateChildrenArray(AXUIElementRef element);
void Describe(AXUIElementRef element);
void FocusApplication(const std::string& application);
std::string GetActiveApplication();
void GetClickableButtons(AXUIElementRef element,
                         std::vector<std::string>& result);
std::vector<std::string> GetClickableButtons();
int GetEditorCursor();
std::string GetEditorSource();
int GetMouseX();
int GetMouseY();
std::string GetRoleDescription(AXUIElementRef element);
std::string GetRawTitle(AXUIElementRef element);
std::vector<std::string> GetRunningApplications();
std::string GetTitle(AXUIElementRef element);
bool HasActionName(AXUIElementRef element, CFStringRef name);
bool IsButton(AXUIElementRef element);
std::string KeyCodeToString(int keyCode, bool shift);
void MouseDown(const std::string& button);
void MouseUp(const std::string& button);
void PressKey(const std::string& key,
              const std::vector<std::string>& modifiers);
void SetEditorState(const std::string& source, int cursor, int cursorEnd);
void SetMouseLocation(int x, int y);
void ToggleKey(const std::string& key,
               const std::vector<std::string>& modifiers, bool down);
void ToLower(std::string& s);
CGKeyCode VirtualKey(const std::string& key, bool shift);

}  // namespace driver
