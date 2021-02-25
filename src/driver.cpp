#include <napi.h>

#include <vector>

#include "driver.hpp"

#if __APPLE__

#include "mac.hpp"

#elif __linux__

#include <X11/Xlib.h>
#include <unistd.h>

#include "linux.hpp"

#else

#include "windows.hpp"

#endif

Napi::Value Click(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  driver::Click(info[0].As<Napi::String>().Utf8Value(),
                info[1].As<Napi::Number>().Int32Value());
  return env.Null();
}

Napi::Value ClickButton(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

#ifdef __APPLE__
  driver::ClickButton(info[0].As<Napi::String>().Utf8Value());
#endif

  return env.Null();
}

Napi::Value FocusApplication(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  driver::FocusApplication(info[0].As<Napi::String>().Utf8Value());
  return env.Null();
}

Napi::Value GetActiveApplication(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), driver::GetActiveApplication());
}

Napi::Value GetClickableButtons(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

#ifdef __APPLE__
  std::vector<std::string> clickable = driver::GetClickableButtons();
  Napi::Array result = Napi::Array::New(env, clickable.size());
  for (size_t i = 0; i < clickable.size(); i++) {
    result[i] = clickable[i];
  }

  return result;
#else
  Napi::Array result = Napi::Array::New(env, 0);
  return result;
#endif
}

Napi::Value GetEditorState(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

#ifdef __APPLE__
  Napi::Object result = Napi::Object::New(env);
  result.Set("text", driver::GetEditorSource());
  result.Set("cursor", driver::GetEditorCursor());
  return result;
#else
  return env.Null();
#endif
}

Napi::Array GetRunningApplications(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::vector<std::string> running = driver::GetRunningApplications();
  Napi::Array result = Napi::Array::New(env, running.size());
  for (size_t i = 0; i < running.size(); i++) {
    result[i] = running[i];
  }

  return result;
}

Napi::Value PressKey(const Napi::CallbackInfo& info) {
#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  Napi::Env env = info.Env();
  std::vector<std::string> modifiers;
  Napi::Array modifierArray = info[1].As<Napi::Array>();
  for (uint32_t i = 0; i < modifierArray.Length(); i++) {
    Napi::Value e = modifierArray[i];
    modifiers.push_back(e.As<Napi::String>().Utf8Value());
  }

#ifdef __linux__
  driver::PressKey(display, info[0].As<Napi::String>().Utf8Value(), modifiers);
#else
  driver::PressKey(info[0].As<Napi::String>().Utf8Value(), modifiers);
#endif

#ifdef __linux__
  XCloseDisplay(display);
  usleep(100000);
#endif

  return env.Null();
}

Napi::Value SetEditorState(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

#ifdef __APPLE__
  driver::SetEditorState(info[0].As<Napi::String>().Utf8Value(),
                         info[1].As<Napi::Number>().Int32Value(),
                         info[2].As<Napi::Number>().Int32Value());
#endif

  return env.Null();
}

Napi::Value SetMouseLocation(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  driver::SetMouseLocation(info[0].As<Napi::Number>().Int32Value(),
                           info[1].As<Napi::Number>().Int32Value());
  return env.Null();
}

Napi::Value TypeText(const Napi::CallbackInfo& info) {
#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  Napi::Env env = info.Env();
  std::vector<std::string> modifiers;
  std::string text = info[0].As<Napi::String>().Utf8Value();
  for (char c : text) {
#ifdef __linux__
    driver::PressKey(display, std::string(1, c), modifiers);
#else
    driver::PressKey(std::string(1, c), modifiers);
#endif
  }

#ifdef __linux__
  XCloseDisplay(display);
#endif

  return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "click"), Napi::Function::New(env, Click));
  exports.Set(Napi::String::New(env, "clickButton"),
              Napi::Function::New(env, ClickButton));
  exports.Set(Napi::String::New(env, "focusApplication"),
              Napi::Function::New(env, FocusApplication));
  exports.Set(Napi::String::New(env, "getActiveApplication"),
              Napi::Function::New(env, GetActiveApplication));
  exports.Set(Napi::String::New(env, "getClickableButtons"),
              Napi::Function::New(env, GetClickableButtons));
  exports.Set(Napi::String::New(env, "getEditorState"),
              Napi::Function::New(env, GetEditorState));
  exports.Set(Napi::String::New(env, "getRunningApplications"),
              Napi::Function::New(env, GetRunningApplications));
  exports.Set(Napi::String::New(env, "pressKey"),
              Napi::Function::New(env, PressKey));
  exports.Set(Napi::String::New(env, "setEditorState"),
              Napi::Function::New(env, SetEditorState));
  exports.Set(Napi::String::New(env, "setMouseLocation"),
              Napi::Function::New(env, SetMouseLocation));
  exports.Set(Napi::String::New(env, "typeText"),
              Napi::Function::New(env, TypeText));
  return exports;
}

NODE_API_MODULE(addon, Init)
