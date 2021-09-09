#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>

#include <string>
#include <tuple>
#include <vector>

#define kVirtualKeyNotFound 65535
#define kAXOpenAction CFSTR("AXOpen")
#define kAXEnhancedUserInterfaceAttribute CFSTR("AXEnhancedUserInterface")
#define kAXManualAccessibilityAttribute CFSTR("AXManualAccessibility")

namespace driver
{

bool ActiveApplicationIsSandboxed();
void Click(const std::string &button, int count);
bool ClickButton(AXUIElementRef element, const std::string &button, int count);
void ClickButton(const std::string &button, int count);
AXUIElementRef CreateActiveTextFieldRef();
AXUIElementRef CreateActiveWindowRef();
CFArrayRef CreateChildrenArray(AXUIElementRef element);
void Describe(AXUIElementRef element);
void FocusApplication(const std::string &application);
std::string GetActiveApplication();
int GetActivePid();
std::tuple<int, int, int, int> GetActiveApplicationWindowBounds();
void GetClickableButtons(AXUIElementRef element,
                         std::vector<std::string> &result);
std::vector<std::string> GetClickableButtons();
std::tuple<std::string, int, bool> GetEditorState();
std::tuple<std::string, int, bool> GetEditorStateFallback(bool paragraph);
std::tuple<int, int> GetMouseLocation();
CFStringRef GetLines(AXUIElementRef element);
CFStringRef GetLineText(AXUIElementRef element, CFMutableArrayRef textChildren);
std::string GetRoleDescription(AXUIElementRef element);
std::string GetRawTitle(AXUIElementRef element);
std::vector<std::string> GetRunningApplications();
std::string GetTitle(AXUIElementRef element);
std::tuple<CGKeyCode, bool, bool> GetVirtualKeyAndModifiers(
    const std::string &key);
bool HasActionName(AXUIElementRef element, CFStringRef name);
bool IsButton(AXUIElementRef element);
void MouseDown(const std::string &button);
void MouseUp(const std::string &button);
void PressKey(const std::string &key,
              const std::vector<std::string> &modifiers);
void SetEditorState(const std::string &text, int cursor, int cursorEnd);
void SetMouseLocation(int x, int y);
void ToggleKey(const std::string &key,
               const std::vector<std::string> &modifiers, bool down);
void ToLower(std::string &s);

} // namespace driver