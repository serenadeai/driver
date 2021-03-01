#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "mac.hpp"

namespace driver {

void Click(const std::string& button, int count) {
  CGEventType downEventType = button == "left" ? kCGEventLeftMouseDown : kCGEventRightMouseDown;
  CGEventType upEventType = button == "left" ? kCGEventLeftMouseUp : kCGEventRightMouseUp;
  CGMouseButton mouseButton = button == "left" ? kCGMouseButtonLeft : kCGMouseButtonRight;

  std::tuple<int, int> location = GetMouseLocation();
  CGEventRef event = CGEventCreateMouseEvent(
      NULL, downEventType, CGPointMake(std::get<0>(location), std::get<1>(location)), mouseButton);
  CGEventPost(kCGHIDEventTap, event);
  CGEventSetType(event, upEventType);
  CGEventPost(kCGHIDEventTap, event);

  for (int i = 0; i < count - 1; i++) {
    CGEventSetIntegerValueField(event, kCGMouseEventClickState, i + 2);

    CGEventSetType(event, downEventType);
    CGEventPost(kCGHIDEventTap, event);

    CGEventSetType(event, upEventType);
    CGEventPost(kCGHIDEventTap, event);
  }

  CFRelease(event);
}

bool ClickButton(AXUIElementRef element, const std::string& button) {
  CFArrayRef children = CreateChildrenArray(element);
  int n = CFArrayGetCount(children);
  for (CFIndex i = 0; i < 20 && i < n; i++) {
    AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
    std::string title = GetTitle(child);
    ToLower(title);
    if (IsButton(child)) {
      if (title == button) {
        if (HasActionName(child, kAXPressAction)) {
          AXUIElementPerformAction(child, kAXPressAction);
          CFRelease(children);
          return true;
        } else if (HasActionName(child, kAXOpenAction)) {
          AXUIElementPerformAction(child, kAXOpenAction);
          CFRelease(children);
          return true;
        }
      }
    } else {
      if (ClickButton(child, button)) {
        CFRelease(children);
        return true;
      }
    }
  }

  CFRelease(children);
  return false;
}

void ClickButton(const std::string& button) {
  AXUIElementRef window = CreateActiveWindowRef();
  if (window == NULL) {
    return;
  }

  std::string name = button;
  ToLower(name);
  ClickButton(window, name);
}

AXUIElementRef CreateActiveWindowRef() {
  NSRunningApplication* running = [NSWorkspace sharedWorkspace].frontmostApplication;
  if (running == NULL) {
    return NULL;
  }

  AXUIElementRef app = AXUIElementCreateApplication(running.processIdentifier);
  AXUIElementRef window = NULL;
  AXUIElementCopyAttributeValue(app, kAXFocusedWindowAttribute,
                                reinterpret_cast<CFTypeRef*>(&window));
  if (window == NULL) {
    return NULL;
  }

  CFRelease(app);
  return static_cast<AXUIElementRef>(window);
}

AXUIElementRef CreateActiveTextFieldRef() {
  NSRunningApplication* running = [NSWorkspace sharedWorkspace].frontmostApplication;
  if (running == NULL) {
    return NULL;
  }

  AXUIElementRef app = AXUIElementCreateApplication(running.processIdentifier);
  CFBooleanRef value = kCFBooleanTrue;
  AXUIElementSetAttributeValue(app, kAXManualAccessibilityAttribute, value);

  AXUIElementRef field = NULL;
  AXUIElementCopyAttributeValue(app, kAXFocusedUIElementAttribute,
                                reinterpret_cast<CFTypeRef*>(&field));
  CFRelease(app);
  if (field == NULL) {
    return NULL;
  }

  std::string role = GetRoleDescription(field);
  if (role == "text field" || role == "text entry area") {
    return field;
  }

  return NULL;
}

CFArrayRef CreateChildrenArray(AXUIElementRef element) {
  CFIndex count;
  AXUIElementGetAttributeValueCount(element, kAXChildrenAttribute, &count);

  CFArrayRef children;
  AXUIElementCopyAttributeValues(element, kAXChildrenAttribute, 0, count, &children);
  return children;
}

void Describe(AXUIElementRef element) {
  const char* delimiter = ",";
  std::cout << "Title: " << GetTitle(element) << std::endl;
  std::cout << "Role Description: " << GetRoleDescription(element) << std::endl;

  CFArrayRef attributes = NULL;
  AXUIElementCopyAttributeNames(element, &attributes);
  if (attributes != NULL) {
    int attributesCount = CFArrayGetCount(attributes);
    std::vector<std::string> attributeStrings;
    for (CFIndex i = 0; i < attributesCount; i++) {
      CFStringRef e = static_cast<CFStringRef>(CFArrayGetValueAtIndex(attributes, i));
      attributeStrings.push_back([(NSString*)e UTF8String]);
    }

    std::ostringstream joinedAttributes;
    std::copy(attributeStrings.begin(), attributeStrings.end(),
              std::ostream_iterator<std::string>(joinedAttributes, delimiter));
    std::cout << "Attributes: " << joinedAttributes.str() << std::endl;
    CFRelease(attributes);
  }

  CFArrayRef actionNames = NULL;
  AXUIElementCopyActionNames(element, &actionNames);
  if (actionNames != NULL) {
    int actionNamesCount = CFArrayGetCount(actionNames);
    std::vector<std::string> actionNameStrings;
    for (CFIndex i = 0; i < actionNamesCount; i++) {
      CFStringRef e = static_cast<CFStringRef>(CFArrayGetValueAtIndex(actionNames, i));
      actionNameStrings.push_back([(NSString*)e UTF8String]);
    }

    std::ostringstream joinedActionNames;
    std::copy(actionNameStrings.begin(), actionNameStrings.end(),
              std::ostream_iterator<std::string>(joinedActionNames, delimiter));
    std::cout << "Action names: " << joinedActionNames.str() << std::endl;
    CFRelease(actionNames);
  }

  std::cout << std::endl;
}

void FocusApplication(const std::string& application) {
  NSString* name = [NSString stringWithCString:application.c_str()
                                      encoding:[NSString defaultCStringEncoding]]
                       .lowercaseString;

  NSMutableSet* pids = [[NSMutableSet alloc] init];
  NSArray* windows = (NSArray*)CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);

  for (NSDictionary* window in windows) {
    [pids addObject:[window objectForKey:@"kCGWindowOwnerPID"]];
  }

  for (NSNumber* pid in [pids allObjects]) {
    NSRunningApplication* app =
        [NSRunningApplication runningApplicationWithProcessIdentifier:[pid intValue]];

    if (app.bundleURL != NULL) {
      if ([app.bundleURL.path.lowercaseString containsString:name]) {
        [app unhide];
        [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
      }
    }
  }
}

std::string GetActiveApplication() {
  NSRunningApplication* running = [NSWorkspace sharedWorkspace].frontmostApplication;
  if (running == NULL) {
    return "";
  }

  if (AXIsProcessTrustedWithOptions(NULL)) {
    AXUIElementRef window = CreateActiveWindowRef();
    if (window != NULL) {
      std::string description = GetRoleDescription(static_cast<AXUIElementRef>(window));
      CFRelease(window);
      if (description == "dialog" || description == "sheet") {
        return "system dialog";
      }
    }
  }

  return [[NSString stringWithFormat:@"%@ %@", running.bundleURL.path, running.bundleIdentifier]
      UTF8String];
}

void GetClickableButtons(AXUIElementRef element, std::vector<std::string>& result) {
  CFArrayRef children = CreateChildrenArray(element);
  int n = CFArrayGetCount(children);
  for (CFIndex i = 0; i < 20 && i < n; i++) {
    AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
    if (IsButton(child)) {
      std::string title = GetTitle(child);
      ToLower(title);
      result.push_back(title);
    } else {
      GetClickableButtons(child, result);
    }
  }

  CFRelease(children);
}

std::vector<std::string> GetClickableButtons() {
  std::vector<std::string> buttons;
  AXUIElementRef window = CreateActiveWindowRef();
  GetClickableButtons(window, buttons);
  CFRelease(window);
  return buttons;
}

int GetEditorCursor() {
  AXUIElementRef field = CreateActiveTextFieldRef();
  if (field == NULL) {
    return 0;
  }

  AXValueRef value = NULL;
  AXUIElementCopyAttributeValue(field, kAXSelectedTextRangeAttribute,
                                reinterpret_cast<CFTypeRef*>(&value));
  if (value == NULL) {
    CFRelease(field);
    return 0;
  }

  CFRange range;
  AXValueGetValue(value, static_cast<AXValueType>(kAXValueCFRangeType), &range);
  int result = range.location + range.length;
  CFRelease(field);
  CFRelease(value);
  return result;
}

std::string GetEditorSource() {
  AXUIElementRef field = CreateActiveTextFieldRef();
  if (field == NULL) {
    return "";
  }

  std::string title = GetTitle(field);
  CFRelease(field);
  return title;
}

std::tuple<int, int> GetMouseLocation() {
  std::tuple<int, int> result;
  CGFloat y = NSEvent.mouseLocation.y;
  if (NSScreen.mainScreen != nil) {
    y = NSScreen.mainScreen.frame.size.height - y;
  }

  std::get<0>(result) = NSEvent.mouseLocation.x;
  std::get<1>(result) = y;
  return result;
}

std::string GetRoleDescription(AXUIElementRef element) {
  CFTypeRef description = NULL;
  AXUIElementCopyAttributeValue(element, kAXRoleDescriptionAttribute,
                                reinterpret_cast<CFTypeRef*>(&description));

  if (description == NULL) {
    return "";
  }

  std::string result([(NSString*)description UTF8String]);
  CFRelease(description);
  return result;
}

std::string GetRawTitle(AXUIElementRef element) {
  CFStringRef title = NULL;
  AXUIElementCopyAttributeValue(element, kAXValueAttribute, reinterpret_cast<CFTypeRef*>(&title));

  if (title == NULL || CFGetTypeID(title) != CFStringGetTypeID()) {
    if (title != NULL) {
      CFRelease(title);
    }

    AXUIElementCopyAttributeValue(element, kAXTitleAttribute, reinterpret_cast<CFTypeRef*>(&title));
  }

  if (title == NULL || CFGetTypeID(title) != CFStringGetTypeID()) {
    if (title != NULL) {
      CFRelease(title);
    }

    return "";
  }

  std::string result([(NSString*)title UTF8String]);
  CFRelease(title);
  return result;
}

std::vector<std::string> GetRunningApplications() {
  std::vector<std::string> result;
  NSMutableSet* pids = [[NSMutableSet alloc] init];
  NSArray* windows = (NSArray*)CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);

  for (NSDictionary* window in windows) {
    [pids addObject:[window objectForKey:@"kCGWindowOwnerPID"]];
  }

  for (NSNumber* pid in [pids allObjects]) {
    NSRunningApplication* app =
        [NSRunningApplication runningApplicationWithProcessIdentifier:[pid intValue]];

    if (app != NULL) {
      result.push_back([app.bundleURL.path UTF8String]);
    }
  }

  return result;
}

std::string GetTitle(AXUIElementRef element) {
  std::string result = GetRawTitle(element);
  if (result == "") {
    CFArrayRef children = CreateChildrenArray(element);
    int n = CFArrayGetCount(children);
    for (CFIndex i = 0; i < 2 && i < n; i++) {
      AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
      std::string inner = GetRawTitle(child);
      if (inner != "") {
        result = inner;
        break;
      }
    }

    CFRelease(children);
  }

  return result;
}

bool HasActionName(AXUIElementRef element, CFStringRef name) {
  CFArrayRef names = NULL;
  AXUIElementCopyActionNames(element, &names);
  if (names == NULL) {
    return false;
  }

  int n = CFArrayGetCount(names);
  for (CFIndex i = 0; i < n; i++) {
    CFStringRef child = static_cast<CFStringRef>(CFArrayGetValueAtIndex(names, i));
    if (CFEqual(child, name)) {
      return true;
    }
  }

  return false;
}

bool IsButton(AXUIElementRef element) {
  return (GetRoleDescription(element) == "button" && GetTitle(element) != "") ||
         HasActionName(element, kAXOpenAction);
}

std::string KeyCodeToString(int keyCode, bool shift) {
  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  CFDataRef layoutData = static_cast<CFDataRef>(
      TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData));
  const UCKeyboardLayout* keyboardLayout =
      reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData));

  UInt32 keysDown = 0;
  UniChar chars[4];
  UniCharCount realLength;
  UInt32 modifiers = shift ? 1 << 1 : 0;
  UCKeyTranslate(keyboardLayout, keyCode, kUCKeyActionDisplay, modifiers, LMGetKbdType(),
                 kUCKeyTranslateNoDeadKeysBit, &keysDown, sizeof(chars) / sizeof(chars[0]),
                 &realLength, chars);

  CFRelease(currentKeyboard);
  NSString* s = static_cast<NSString*>(CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1));
  std::string result([s UTF8String]);
  CFRelease(s);
  return result;
}

void MouseDown(const std::string& button) {
  std::tuple<int, int> location = GetMouseLocation();
  CGEventRef event = CGEventCreateMouseEvent(
      NULL, button == "left" ? kCGEventLeftMouseDown : kCGEventRightMouseDown,
      CGPointMake(std::get<0>(location), std::get<1>(location)),
      button == "left" ? kCGMouseButtonLeft : kCGMouseButtonRight);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
}

void MouseUp(const std::string& button) {
  std::tuple<int, int> location = GetMouseLocation();
  CGEventRef event =
      CGEventCreateMouseEvent(NULL, button == "left" ? kCGEventLeftMouseUp : kCGEventRightMouseUp,
                              CGPointMake(std::get<0>(location), std::get<1>(location)),
                              button == "left" ? kCGMouseButtonLeft : kCGMouseButtonRight);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
}

void PressKey(const std::string& key, const std::vector<std::string>& modifiers) {
  ToggleKey(key, modifiers, true);
  ToggleKey(key, modifiers, false);
}

void SetEditorState(const std::string& source, int cursor, int cursorEnd) {
  AXUIElementRef field = CreateActiveTextFieldRef();
  if (field == NULL) {
    return;
  }

  CFStringRef sourceValue = CFStringCreateWithCString(NULL, source.c_str(), kCFStringEncodingUTF8);
  CFTypeRef value = NULL;
  AXUIElementCopyAttributeValue(field, kAXValueAttribute, reinterpret_cast<CFTypeRef*>(&value));
  if (value != NULL) {
    CFRelease(value);
    AXUIElementSetAttributeValue(field, kAXValueAttribute, sourceValue);
  } else {
    AXUIElementSetAttributeValue(field, kAXTitleAttribute, sourceValue);
  }

  int length = 0;
  if (cursorEnd > 0) {
    length = cursorEnd - cursor;
  }

  CFRange range = CFRangeMake(cursor, length);
  AXValueRef rangeValue = AXValueCreate(static_cast<AXValueType>(kAXValueCFRangeType), &range);
  AXUIElementSetAttributeValue(field, kAXSelectedTextRangeAttribute, rangeValue);

  CFRelease(field);
  CFRelease(sourceValue);
  CFRelease(rangeValue);
}

void SetMouseLocation(int x, int y) {
  CGEventRef event =
      CGEventCreateMouseEvent(nil, kCGEventMouseMoved, CGPointMake(x, y), kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
  usleep(100000);
}

void ToggleKey(const std::string& key, const std::vector<std::string>& modifiers, bool down) {
  std::vector<std::string> adjustedModifiers = modifiers;
  CGKeyCode virtualKey = VirtualKey(key, false);
  if (virtualKey == kVirtualKeyNotFound) {
    virtualKey = VirtualKey(key, true);
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "shift") ==
        adjustedModifiers.end()) {
      adjustedModifiers.push_back("shift");
    }
  }

  CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
  CGEventRef event = CGEventCreateKeyboardEvent(source, virtualKey, down);

  if (virtualKey == kVirtualKeyNotFound) {
    unichar c[1];
    c[0] = [[NSString stringWithFormat:@"%c", key[0]] characterAtIndex:0];
    CGEventKeyboardSetUnicodeString(event, 1, c);
  }

  CGEventSetFlags(event, 0);
  if (down) {
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "shift") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskShift);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "control") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskControl);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "ctrl") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskControl);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "alt") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskAlternate);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "option") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskAlternate);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "command") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskCommand);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "cmd") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskCommand);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "function") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskSecondaryFn);
    }
    if (std::find(adjustedModifiers.begin(), adjustedModifiers.end(), "fn") !=
        adjustedModifiers.end()) {
      CGEventSetFlags(event, CGEventGetFlags(event) | kCGEventFlagMaskSecondaryFn);
    }
  }

  CGEventPost(kCGHIDEventTap, event);
  CFRelease(source);
  CFRelease(event);
  usleep(1000);
}

void ToLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

CGKeyCode VirtualKey(const std::string& key, bool shift) {
  if (key == "enter") {
    return CGKeyCode(kVK_Return);
  } else if (key == "return") {
    return CGKeyCode(kVK_Return);
  } else if (key == "tab") {
    return CGKeyCode(kVK_Tab);
  } else if (key == "space") {
    return CGKeyCode(kVK_Space);
  } else if (key == "backspace") {
    return CGKeyCode(kVK_Delete);
  } else if (key == "delete") {
    return CGKeyCode(kVK_Delete);
  } else if (key == "forwarddelete") {
    return CGKeyCode(kVK_ForwardDelete);
  } else if (key == "escape") {
    return CGKeyCode(kVK_Escape);
  } else if (key == "command" || key == "cmd") {
    return CGKeyCode(kVK_Command);
  } else if (key == "caps") {
    return CGKeyCode(kVK_CapsLock);
  } else if (key == "shift") {
    return CGKeyCode(kVK_Shift);
  } else if (key == "option" || key == "alt") {
    return CGKeyCode(kVK_Option);
  } else if (key == "alt") {
    return CGKeyCode(kVK_Option);
  } else if (key == "control" || key == "ctrl") {
    return CGKeyCode(kVK_Control);
  } else if (key == "function" || key == "fn") {
    return CGKeyCode(kVK_Function);
  } else if (key == "home") {
    return CGKeyCode(kVK_Home);
  } else if (key == "pageup") {
    return CGKeyCode(kVK_PageUp);
  } else if (key == "end") {
    return CGKeyCode(kVK_End);
  } else if (key == "pagedown") {
    return CGKeyCode(kVK_PageDown);
  } else if (key == "left") {
    return CGKeyCode(kVK_LeftArrow);
  } else if (key == "right") {
    return CGKeyCode(kVK_RightArrow);
  } else if (key == "down") {
    return CGKeyCode(kVK_DownArrow);
  } else if (key == "up") {
    return CGKeyCode(kVK_UpArrow);
  } else if (key == "f1") {
    return CGKeyCode(kVK_F1);
  } else if (key == "f2") {
    return CGKeyCode(kVK_F2);
  } else if (key == "f3") {
    return CGKeyCode(kVK_F3);
  } else if (key == "f4") {
    return CGKeyCode(kVK_F4);
  } else if (key == "f5") {
    return CGKeyCode(kVK_F5);
  } else if (key == "f6") {
    return CGKeyCode(kVK_F6);
  } else if (key == "f7") {
    return CGKeyCode(kVK_F7);
  } else if (key == "f8") {
    return CGKeyCode(kVK_F8);
  } else if (key == "f9") {
    return CGKeyCode(kVK_F9);
  } else if (key == "f10") {
    return CGKeyCode(kVK_F10);
  } else if (key == "f11") {
    return CGKeyCode(kVK_F11);
  } else if (key == "f12") {
    return CGKeyCode(kVK_F12);
  }

  for (int i = 0; i < 128; i++) {
    if (key == KeyCodeToString(i, shift)) {
      return CGKeyCode(i);
    }
  }

  return CGKeyCode(kVirtualKeyNotFound);
}

}  // namespace driver
