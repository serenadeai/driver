#include <napi.h>

#include <tuple>
#include <vector>

#include "driver.hpp"

#ifdef __APPLE__
#include "mac.hpp"
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <unistd.h>

#include "linux.hpp"
#endif

#ifdef _WIN32
#include "windows.hpp"
#endif

#ifdef __APPLE__
#define AUTORELEASE(statement) \
  {                            \
    @autoreleasepool {         \
      statement;               \
    }                          \
  }
#else
#define AUTORELEASE(statement) \
  { statement; }
#endif

Napi::Promise Click(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::string button = info[0].As<Napi::String>().Utf8Value();
  int count = info[1].As<Napi::Number>().Int32Value();
  if (count < 1) {
    deferred.Resolve(env.Undefined());
    return deferred.Promise();
  }

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  driver::Click(display, button, count);
  XCloseDisplay(display);
#else
  AUTORELEASE(driver::Click(button, count));
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise ClickButton(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  int count = info[1].As<Napi::Number>().Int32Value();
  if (count < 1) {
    deferred.Resolve(env.Undefined());
    return deferred.Promise();
  }

#ifdef __APPLE__
  AUTORELEASE(driver::ClickButton(info[0].As<Napi::String>().Utf8Value(), count));
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise FocusApplication(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  AUTORELEASE(driver::FocusApplication(info[0].As<Napi::String>().Utf8Value()));

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise GetActiveApplication(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::string ret;
  AUTORELEASE(ret = driver::GetActiveApplication());

  deferred.Resolve(Napi::String::New(env, ret));
  return deferred.Promise();
}

Napi::Promise GetClickableButtons(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

#ifdef __APPLE__
  std::vector<std::string> clickable;
  AUTORELEASE(clickable = driver::GetClickableButtons());

  Napi::Array result = Napi::Array::New(env, clickable.size());
  for (size_t i = 0; i < clickable.size(); i++) {
    result[i] = clickable[i];
  }

  deferred.Resolve(result);
#else
  Napi::Array result = Napi::Array::New(env, 0);
  deferred.Resolve(result);
#endif

  return deferred.Promise();
}

Napi::Promise GetEditorState(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  std::tuple<std::string, int, bool> state = driver::GetEditorState(display);
  XCloseDisplay(display);
#else
  std::tuple<std::string, int, bool> state;
  AUTORELEASE(state = driver::GetEditorState());
#endif

  Napi::Object result = Napi::Object::New(env);
  result.Set("text", std::get<0>(state));
  result.Set("cursor", std::get<1>(state));
  result.Set("error", std::get<2>(state));
  deferred.Resolve(result);

  return deferred.Promise();
}

Napi::Promise GetEditorStateFallback(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  bool paragraph = info[0].As<Napi::Boolean>().Value();

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  std::tuple<std::string, int, bool> state = driver::GetEditorStateFallback(display, paragraph);
  XCloseDisplay(display);
#else
  std::tuple<std::string, int, bool> state;
  AUTORELEASE(state = driver::GetEditorStateFallback(paragraph));
#endif

  Napi::Object result = Napi::Object::New(env);
  result.Set("text", std::get<0>(state));
  result.Set("cursor", std::get<1>(state));
  result.Set("error", std::get<2>(state));
  deferred.Resolve(result);

  return deferred.Promise();
}

Napi::Promise GetMouseLocation(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::tuple<int, int> location;
  AUTORELEASE(location = driver::GetMouseLocation());
  Napi::Object result = Napi::Object::New(env);
  result.Set("x", std::get<0>(location));
  result.Set("y", std::get<1>(location));
  deferred.Resolve(result);

  return deferred.Promise();
}

Napi::Promise GetRunningApplications(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::vector<std::string> running;
  AUTORELEASE(running = driver::GetRunningApplications());

  Napi::Array result = Napi::Array::New(env, running.size());
  for (size_t i = 0; i < running.size(); i++) {
    result[i] = running[i];
  }

  deferred.Resolve(result);
  return deferred.Promise();
}

Napi::Promise KeyDown(const Napi::CallbackInfo& info) { return ToggleKey(info, true); }

Napi::Promise KeyUp(const Napi::CallbackInfo& info) { return ToggleKey(info, false); }

Napi::Promise MouseDown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  driver::MouseDown(display, info[0].As<Napi::String>().Utf8Value());
  XCloseDisplay(display);
#else
  AUTORELEASE(driver::MouseDown(info[0].As<Napi::String>().Utf8Value()));
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise MouseUp(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  driver::MouseUp(display, info[0].As<Napi::String>().Utf8Value());
  XCloseDisplay(display);
#else
  AUTORELEASE(driver::MouseUp(info[0].As<Napi::String>().Utf8Value()));
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise PressKey(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  int count = info[3].As<Napi::Number>().Int32Value();
  if (count < 1) {
    deferred.Resolve(env.Undefined());
    return deferred.Promise();
  }

  std::vector<std::string> modifiers;
  ToStringVector(info[1].As<Napi::Array>(), modifiers);

  std::vector<std::string> stickyModifiers;
  ToStringVector(info[2].As<Napi::Array>(), stickyModifiers);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  for (int i = 0; i < count; i++) {
#ifdef __linux__
    driver::PressKey(display, info[0].As<Napi::String>().Utf8Value(), modifiers, stickyModifiers);
    usleep(100000);
#else
    AUTORELEASE(
        driver::PressKey(info[0].As<Napi::String>().Utf8Value(), modifiers, stickyModifiers));
#endif
  }

#ifdef __linux__
  XCloseDisplay(display);
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise SetEditorState(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

#ifdef __APPLE__
  AUTORELEASE(driver::SetEditorState(info[0].As<Napi::String>().Utf8Value(),
                                     info[1].As<Napi::Number>().Int32Value(),
                                     info[2].As<Napi::Number>().Int32Value()));
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise SetMouseLocation(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  AUTORELEASE(driver::SetMouseLocation(info[0].As<Napi::Number>().Int32Value(),
                                       info[1].As<Napi::Number>().Int32Value()));

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Promise ToggleKey(const Napi::CallbackInfo& info, bool down) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::vector<std::string> stickyModifiers;
  ToStringVector(info[1].As<Napi::Array>(), stickyModifiers);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  driver::ToggleKey(display, info[0].As<Napi::String>().Utf8Value(), down);
  usleep(100000);
  XCloseDisplay(display);
#endif

#ifdef __APPLE__
  AUTORELEASE(driver::ToggleKey(info[0].As<Napi::String>().Utf8Value(), std::vector<std::string>{},
                                stickyModifiers, down));
#endif

#ifdef _WIN32
  driver::ToggleKey(info[0].As<Napi::String>().Utf8Value(), stickyModifiers, down);
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

void ToStringVector(const Napi::Array input, std::vector<std::string>& output) {
  for (uint32_t i = 0; i < input.Length(); i++) {
    Napi::Value e = input[i];
    output.push_back(e.As<Napi::String>().Utf8Value());
  }
}

Napi::Promise TypeText(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::string text = info[0].As<Napi::String>().Utf8Value();
  std::vector<std::string> stickyModifiers;
  ToStringVector(info[1].As<Napi::Array>(), stickyModifiers);

#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  for (char c : text) {
#ifdef __linux__
    driver::PressKey(display, std::string(1, c), std::vector<std::string>{});
#endif
#ifdef __APPLE__
    AUTORELEASE(driver::PressKey(std::string(1, c), std::vector<std::string>{},
                                 stickyModifiers));
#endif
#ifdef _WIN32
    driver::PressKey(std::string(1, c), std::vector<std::string>{}, stickyModifiers);
#endif
  }

#ifdef __linux__
  XCloseDisplay(display);
#endif

  deferred.Resolve(env.Undefined());
  return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "click"), Napi::Function::New(env, Click));
  exports.Set(Napi::String::New(env, "clickButton"), Napi::Function::New(env, ClickButton));
  exports.Set(Napi::String::New(env, "focusApplication"),
              Napi::Function::New(env, FocusApplication));
  exports.Set(Napi::String::New(env, "getActiveApplication"),
              Napi::Function::New(env, GetActiveApplication));
  exports.Set(Napi::String::New(env, "getClickableButtons"),
              Napi::Function::New(env, GetClickableButtons));
  exports.Set(Napi::String::New(env, "getEditorState"), Napi::Function::New(env, GetEditorState));
  exports.Set(Napi::String::New(env, "getEditorStateFallback"),
              Napi::Function::New(env, GetEditorStateFallback));
  exports.Set(Napi::String::New(env, "getMouseLocation"),
              Napi::Function::New(env, GetMouseLocation));
  exports.Set(Napi::String::New(env, "getRunningApplications"),
              Napi::Function::New(env, GetRunningApplications));
  exports.Set(Napi::String::New(env, "keyDown"), Napi::Function::New(env, KeyDown));
  exports.Set(Napi::String::New(env, "keyUp"), Napi::Function::New(env, KeyUp));
  exports.Set(Napi::String::New(env, "pressKey"), Napi::Function::New(env, PressKey));
  exports.Set(Napi::String::New(env, "mouseDown"), Napi::Function::New(env, MouseDown));
  exports.Set(Napi::String::New(env, "mouseUp"), Napi::Function::New(env, MouseUp));
  exports.Set(Napi::String::New(env, "setEditorState"), Napi::Function::New(env, SetEditorState));
  exports.Set(Napi::String::New(env, "setMouseLocation"),
              Napi::Function::New(env, SetMouseLocation));
  exports.Set(Napi::String::New(env, "typeText"), Napi::Function::New(env, TypeText));

  return exports;
}

NODE_API_MODULE(addon, Init)
