#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>
#include <vector>

#define kVirtualKeyNotFound 65535

namespace driver {

std::string KeyCodeToString(int keyCode, bool shift) {
  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  CFDataRef layoutData =
      (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
  const UCKeyboardLayout* keyboardLayout = (const UCKeyboardLayout*)CFDataGetBytePtr(layoutData);

  UInt32 keysDown = 0;
  UniChar chars[4];
  UniCharCount realLength;
  UInt32 modifiers = shift ? 1 << 1 : 0;
  UCKeyTranslate(keyboardLayout, keyCode, kUCKeyActionDisplay, modifiers, LMGetKbdType(),
                 kUCKeyTranslateNoDeadKeysBit, &keysDown, sizeof(chars) / sizeof(chars[0]),
                 &realLength, chars);

  CFRelease(currentKeyboard);
  return [(NSString*)CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1) UTF8String];
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

void Click(const std::string& buttonType, int count) {
  CGEventType downEventType = buttonType == "left" ? kCGEventLeftMouseDown : kCGEventRightMouseDown;
  CGEventType upEventType = buttonType == "left" ? kCGEventLeftMouseUp : kCGEventRightMouseUp;
  CGMouseButton button = buttonType == "left" ? kCGMouseButtonLeft : kCGMouseButtonRight;

  CGFloat y = NSEvent.mouseLocation.y;
  if (NSScreen.mainScreen != nil) {
    y = NSScreen.mainScreen.frame.size.height - y;
  }

  CGEventRef event =
      CGEventCreateMouseEvent(NULL, downEventType, CGPointMake(NSEvent.mouseLocation.x, y), button);
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
  NSRunningApplication* app = [NSWorkspace sharedWorkspace].frontmostApplication;
  if (app == NULL) {
    return "";
  }

  return
      [[NSString stringWithFormat:@"%@ %@", app.bundleURL.path, app.bundleIdentifier] UTF8String];
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

void PressKey(const std::string& key, const std::vector<std::string>& modifiers) {
  ToggleKey(key, modifiers, true);
  ToggleKey(key, modifiers, false);
}

void SetMouseLocation(int x, int y) {
  CGEventRef event =
      CGEventCreateMouseEvent(nil, kCGEventMouseMoved, CGPointMake(x, y), kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
}

}  // namespace driver
