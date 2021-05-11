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

bool ClickButton(AXUIElementRef element, const std::string& button, int count) {
  CFArrayRef children = CreateChildrenArray(element);
  if (children == NULL) {
    return false;
  }

  int n = CFArrayGetCount(children);
  for (CFIndex i = 0; i < 20 && i < n; i++) {
    AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
    std::string title = GetTitle(child);
    ToLower(title);
    if (IsButton(child)) {
      if (title == button) {
        if (HasActionName(child, kAXPressAction)) {
          for (int i = 0; i < count; i++) {
            AXUIElementPerformAction(child, kAXPressAction);
          }
          CFRelease(children);
          return true;
        } else if (HasActionName(child, kAXOpenAction)) {
          for (int i = 0; i < count; i++) {
            AXUIElementPerformAction(child, kAXOpenAction);
          }
          CFRelease(children);
          return true;
        }
      }
    } else {
      if (ClickButton(child, button, count)) {
        CFRelease(children);
        return true;
      }
    }
  }

  CFRelease(children);
  return false;
}

void ClickButton(const std::string& button, int count) {
  if (!AXIsProcessTrustedWithOptions(NULL)) {
    return;
  }

  AXUIElementRef window = CreateActiveWindowRef();
  if (window == NULL) {
    return;
  }

  std::string name = button;
  ToLower(name);
  ClickButton(window, name, count);
}

AXUIElementRef CreateActiveWindowRef() {
  int pid = GetActivePid();
  if (pid == -1) {
    return NULL;
  }

  AXUIElementRef app = AXUIElementCreateApplication(pid);
  if (app == NULL) {
    return NULL;
  }

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
  int pid = GetActivePid();
  if (pid == -1) {
    return NULL;
  }

  AXUIElementRef app = AXUIElementCreateApplication(pid);
  if (app == NULL) {
    return NULL;
  }

  AXUIElementSetAttributeValue(app, kAXManualAccessibilityAttribute, kCFBooleanTrue);
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
  if (element == NULL) {
    return NULL;
  }

  CFIndex count;
  CFArrayRef children;
  AXUIElementGetAttributeValueCount(element, kAXChildrenAttribute, &count);
  AXUIElementCopyAttributeValues(element, kAXChildrenAttribute, 0, count, &children);
  return children;
}

CFStringRef GetLineText(AXUIElementRef element, CFMutableArrayRef textChildren) {
  if (element == NULL) {
    return NULL;
  }

  CFIndex count;
  AXUIElementGetAttributeValueCount(element, kAXChildrenAttribute, &count);

  if (count == 0) {
    CFStringRef value;
    AXUIElementCopyAttributeValue(element, kAXValueAttribute, reinterpret_cast<CFTypeRef*>(&value));
    CFArrayAppendValue(textChildren, value);
  } else {
    CFArrayRef children;
    AXUIElementCopyAttributeValues(element, kAXChildrenAttribute, 0, count, &children);
    for (CFIndex i = 0; i < count; i++) {
      AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
      CFStringRef currText = GetLineText(child, textChildren);
      CFRelease(currText);
    }
    CFRelease(children);
  }

  return CFStringCreateByCombiningStrings(kCFAllocatorDefault, textChildren, CFSTR(""));
}

CFStringRef GetLines(AXUIElementRef element) {
  if (element == NULL) {
    return NULL;
  }

  CFIndex count;
  CFArrayRef children;
  AXUIElementGetAttributeValueCount(element, kAXChildrenAttribute, &count);

  if (count == 0) {
    CFStringRef value;
    AXUIElementCopyAttributeValue(element, kAXValueAttribute, reinterpret_cast<CFTypeRef*>(&value));
    return value;
  } else {
    CFMutableArrayRef lines = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    AXUIElementCopyAttributeValues(element, kAXChildrenAttribute, 0, count, &children);
    for (CFIndex i = 0; i < count; i++) {
      AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(children, i));
      CFMutableArrayRef lineTextChildren = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
      if (GetRoleDescription(child) == "list") {
        // Recurse one level to get each item in the list
        CFArrayAppendValue(lines, GetLines(child));
      } else {
        CFArrayAppendValue(lines, GetLineText(child, lineTextChildren));
      }
      CFRelease(lineTextChildren);
    }
    CFStringRef combined =
        CFStringCreateByCombiningStrings(kCFAllocatorDefault, lines, CFSTR("\n"));
    CFRelease(lines);
    return combined;
  }
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

  int pid = GetActivePid();
  if (pid == -1) {
    return "";
  }

  NSRunningApplication* running =
      [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
  return [[NSString stringWithFormat:@"%@", running.bundleURL.path] UTF8String];
}

bool ActiveApplicationIsSandboxed() {
  int pid = GetActivePid();
  NSRunningApplication* running = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
  NSURL *bundleURL = running.bundleURL;
  SecStaticCodeRef staticCode = NULL;
  bool isSandboxed = false;

  if (SecStaticCodeCreateWithPath((__bridge CFURLRef)bundleURL, kSecCSDefaultFlags, &staticCode) == errSecSuccess) {
    if (SecStaticCodeCheckValidityWithErrors(staticCode, kSecCSBasicValidateOnly, NULL, NULL) == errSecSuccess) {
      SecRequirementRef sandboxRequirement;
      if (SecRequirementCreateWithString(CFSTR("entitlement[\"com.apple.security.app-sandbox\"] exists"), kSecCSDefaultFlags,
                                    &sandboxRequirement) == errSecSuccess)
      {
        OSStatus codeCheckResult = SecStaticCodeCheckValidityWithErrors(staticCode, kSecCSBasicValidateOnly, sandboxRequirement, NULL);
        if (codeCheckResult == errSecSuccess) {
            isSandboxed = true;
        }
      }
    }
    CFRelease(staticCode);
  }
  return isSandboxed;
}

int GetActivePid() {
  NSArray* windows = (NSArray*)CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
  for (NSDictionary* window in windows) {
    int pid = [[window objectForKey:@"kCGWindowOwnerPID"] intValue];
    if ([NSRunningApplication runningApplicationWithProcessIdentifier:pid].active) {
      return pid;
    }
  }

  NSRunningApplication* running = [NSWorkspace sharedWorkspace].frontmostApplication;
  if (running == NULL) {
    return -1;
  }

  return running.processIdentifier;
}

void GetClickableButtons(AXUIElementRef element, std::vector<std::string>& result) {
  CFArrayRef children = CreateChildrenArray(element);
  if (children == NULL) {
    return;
  }

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
  std::vector<std::string> result;
  if (!AXIsProcessTrustedWithOptions(NULL)) {
    return result;
  }

  AXUIElementRef window = CreateActiveWindowRef();
  GetClickableButtons(window, result);
  CFRelease(window);
  return result;
}

std::tuple<std::string, int, bool> GetEditorState() {
  std::tuple<std::string, int, bool> result;
  std::get<2>(result) = true;

  AXUIElementRef field = NULL;
  if (AXIsProcessTrustedWithOptions(NULL)) {
    field = CreateActiveTextFieldRef();
  }

  if (field == NULL) {
    return result;
  }

  AXValueRef value = NULL;
  AXUIElementCopyAttributeValue(field, kAXSelectedTextRangeAttribute,
                                reinterpret_cast<CFTypeRef*>(&value));
  std::string activeApp = GetActiveApplication();
  if (activeApp.find("Slack") != std::string::npos &&
      GetRoleDescription(field) == "text entry area") {
    // Can't obtain cursor position in App Store version of Slack
    if (ActiveApplicationIsSandboxed()) {
      return result;
    }
    CFStringRef sourceRef = GetLines(field);
    NSData* sourceData = [(NSString*)sourceRef dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
    CFRelease(sourceRef);
    std::wstring source(static_cast<const wchar_t*>([sourceData bytes]),
                        [sourceData length] / sizeof(wchar_t));
    std::string narrow(source.begin(), source.end());

    // Bulleted lists in the Slack app send these characters in place of the bullet
    // 3n + 1-th level: u\0006 (acknowledge)
    // 3n + 2-th level: u\0007 (bell)
    // 3n-th level: \t (tab)
    // We replace the first two with a space to maintain the proper cursor position
    for (CFIndex i = 0; i < (int)narrow.size(); i++) {
      if (narrow[i] == '\u0006' || narrow[i] == '\u0007') {
        narrow[i] = ' ';
      }
    }

    std::get<0>(result) = narrow;
    if (value != NULL) {
      CFRange range;
      AXValueGetValue(value, static_cast<AXValueType>(kAXValueCFRangeType), &range);
      int newLineCount = 0;
      for (CFIndex i = 0; (i < range.location + newLineCount) && (i < (int)narrow.size()); i++) {
        if (narrow[i] == '\n') {
          // Double newlines update the range.location property correctly, so avoid double counting
          if (i > 0 && narrow[i - 1] != '\n') {
            newLineCount++;
          }
        }
      }
      std::get<1>(result) = std::min(range.location + newLineCount, (long)narrow.size());
      CFRelease(value);
    }
  } else {
    std::get<0>(result) = GetTitle(field);
    if (value != NULL) {
      CFRange range;
      AXValueGetValue(value, static_cast<AXValueType>(kAXValueCFRangeType), &range);
      std::get<1>(result) = range.location;
      CFRelease(value);
    }
  }

  std::get<2>(result) = false;
  CFRelease(field);
  return result;
}

std::tuple<std::string, int, bool> GetEditorStateFallback() {
  std::tuple<std::string, int, bool> result;

  NSPasteboard* pasteboard = NSPasteboard.generalPasteboard;
  [pasteboard declareTypes:@[ NSPasteboardTypeString ] owner:NULL];
  NSString* previous = @"";
  if (pasteboard.pasteboardItems.count > 0) {
    previous = [pasteboard.pasteboardItems[0] stringForType:NSPasteboardTypeString];
  }

  PressKey("left", std::vector<std::string>{"command", "shift"});
  PressKey("up", std::vector<std::string>{"command", "shift"});
  PressKey("c", std::vector<std::string>{"command"});
  usleep(10000);
  PressKey("right", std::vector<std::string>{});
  NSString* left = [pasteboard.pasteboardItems[0] stringForType:NSPasteboardTypeString];

  PressKey("right", std::vector<std::string>{"command", "shift"});
  PressKey("down", std::vector<std::string>{"command", "shift"});
  PressKey("c", std::vector<std::string>{"command"});
  usleep(10000);
  PressKey("left", std::vector<std::string>{});
  NSString* right = [pasteboard.pasteboardItems[0] stringForType:NSPasteboardTypeString];

  [pasteboard setString:previous forType:NSPasteboardTypeString];
  std::get<0>(result) = [[NSString stringWithFormat:@"%@%@", left, right] UTF8String];
  std::get<1>(result) = left.length;
  std::get<2>(result) = false;
  return result;
}

std::tuple<int, int> GetMouseLocation() {
  std::tuple<int, int> result;
  std::get<0>(result) = NSEvent.mouseLocation.x;

  NSScreen *smallestScreen = NSScreen.mainScreen;
  NSEnumerator *screens = [NSScreen.screens objectEnumerator];
  NSScreen *screen;
  while ((screen = screens.nextObject)){
    if (screen.frame.size.height <= smallestScreen.frame.size.height) {
      smallestScreen = screen;
    }
  };
  CGFloat y = NSEvent.mouseLocation.y;
  if (smallestScreen != nil) {
    y = smallestScreen.frame.size.height - y;
  }
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
  if (element == NULL) {
    return "";
  }

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
    if (children == NULL) {
      return result;
    }

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

std::tuple<CGKeyCode, bool, bool> GetVirtualKeyAndModifiers(const std::string& key) {
  std::tuple<CGKeyCode, bool, bool> result;
  std::get<0>(result) = kVirtualKeyNotFound;
  if (key == "") {
    return result;
  }

  if (key == "enter" || key == "\n" || key == "return") {
    std::get<0>(result) = CGKeyCode(kVK_Return);
  } else if (key == "tab" || key == "\t") {
    std::get<0>(result) = CGKeyCode(kVK_Tab);
  } else if (key == "space" || key == " ") {
    std::get<0>(result) = CGKeyCode(kVK_Space);
  } else if (key == "backspace" || key == "delete") {
    std::get<0>(result) = CGKeyCode(kVK_Delete);
  } else if (key == "forwarddelete") {
    std::get<0>(result) = CGKeyCode(kVK_ForwardDelete);
  } else if (key == "escape") {
    std::get<0>(result) = CGKeyCode(kVK_Escape);
  } else if (key == "command" || key == "cmd" || key == "commandOrControl") {
    std::get<0>(result) = CGKeyCode(kVK_Command);
  } else if (key == "caps") {
    std::get<0>(result) = CGKeyCode(kVK_CapsLock);
  } else if (key == "shift") {
    std::get<0>(result) = CGKeyCode(kVK_Shift);
  } else if (key == "option" || key == "alt") {
    std::get<0>(result) = CGKeyCode(kVK_Option);
  } else if (key == "control" || key == "ctrl") {
    std::get<0>(result) = CGKeyCode(kVK_Control);
  } else if (key == "function" || key == "fn") {
    std::get<0>(result) = CGKeyCode(kVK_Function);
  } else if (key == "home") {
    std::get<0>(result) = CGKeyCode(kVK_Home);
  } else if (key == "pageup") {
    std::get<0>(result) = CGKeyCode(kVK_PageUp);
  } else if (key == "end") {
    std::get<0>(result) = CGKeyCode(kVK_End);
  } else if (key == "pagedown") {
    std::get<0>(result) = CGKeyCode(kVK_PageDown);
  } else if (key == "left") {
    std::get<0>(result) = CGKeyCode(kVK_LeftArrow);
  } else if (key == "right") {
    std::get<0>(result) = CGKeyCode(kVK_RightArrow);
  } else if (key == "down") {
    std::get<0>(result) = CGKeyCode(kVK_DownArrow);
  } else if (key == "up") {
    std::get<0>(result) = CGKeyCode(kVK_UpArrow);
  } else if (key == "f1") {
    std::get<0>(result) = CGKeyCode(kVK_F1);
  } else if (key == "f2") {
    std::get<0>(result) = CGKeyCode(kVK_F2);
  } else if (key == "f3") {
    std::get<0>(result) = CGKeyCode(kVK_F3);
  } else if (key == "f4") {
    std::get<0>(result) = CGKeyCode(kVK_F4);
  } else if (key == "f5") {
    std::get<0>(result) = CGKeyCode(kVK_F5);
  } else if (key == "f6") {
    std::get<0>(result) = CGKeyCode(kVK_F6);
  } else if (key == "f7") {
    std::get<0>(result) = CGKeyCode(kVK_F7);
  } else if (key == "f8") {
    std::get<0>(result) = CGKeyCode(kVK_F8);
  } else if (key == "f9") {
    std::get<0>(result) = CGKeyCode(kVK_F9);
  } else if (key == "f10") {
    std::get<0>(result) = CGKeyCode(kVK_F10);
  } else if (key == "f11") {
    std::get<0>(result) = CGKeyCode(kVK_F11);
  } else if (key == "f12") {
    std::get<0>(result) = CGKeyCode(kVK_F12);
  }

  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  CFDataRef layoutData = static_cast<CFDataRef>(
      TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData));
  const UCKeyboardLayout* keyboardLayout =
      reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData));

  if (std::get<0>(result) == kVirtualKeyNotFound) {
    for (int shift = 0; shift < 2; shift++) {
      for (int altgr = 0; altgr < 2; altgr++) {
        for (int i = 0; i < 128; i++) {
          UInt32 keysDown = 0;
          UniChar chars[4];
          UniCharCount realLength;
          UInt32 modifiers = 0;
          if (shift == 1) {
            modifiers |= shiftKey >> 8;
          }
          if (altgr == 1) {
            modifiers |= optionKey >> 8;
          }

          UCKeyTranslate(keyboardLayout, CGKeyCode(i), kUCKeyActionDisplay, modifiers,
                         LMGetKbdType(), kUCKeyTranslateNoDeadKeysBit, &keysDown,
                         sizeof(chars) / sizeof(chars[0]), &realLength, chars);

          NSString* s =
              static_cast<NSString*>(CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1));
          if (std::string([s UTF8String]) == key) {
            std::get<0>(result) = CGKeyCode(i);
            std::get<1>(result) = shift == 1;
            std::get<2>(result) = altgr == 1;
            CFRelease(s);
            CFRelease(currentKeyboard);
            return result;
          }

          CFRelease(s);
        }
      }
    }
  }

  CFRelease(currentKeyboard);
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
  if (AXIsProcessTrustedWithOptions(NULL)) {
    return;
  }

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
  std::tuple<CGKeyCode, bool, bool> keycode = GetVirtualKeyAndModifiers(key);
  if (std::get<1>(keycode)) {
    adjustedModifiers.push_back("shift");
  }
  if (std::get<2>(keycode)) {
    adjustedModifiers.push_back("option");
  }

  CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
  CGEventRef event = CGEventCreateKeyboardEvent(source, std::get<0>(keycode), down);

  if (std::get<0>(keycode) == kVirtualKeyNotFound && key != "") {
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

  // determined empirically by typing into a variety of applications
  usleep(4000);
}

void ToLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

}  // namespace driver
