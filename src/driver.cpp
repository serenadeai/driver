#include <napi.h>

#include <vector>

#if __APPLE__
#include <unistd.h>
#include "lib/mac.hpp"
#elif __linux__
#include <X11/Xlib.h>
#include <unistd.h>
#include "lib/linux.hpp"
#else
#include <Windows.h>
#include "lib/windows.hpp"
#endif

Napi::Value FocusApplication(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  driver::FocusApplication(info[0].As<Napi::String>().Utf8Value());
  return env.Null();
}

Napi::Value GetActiveApplication(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), driver::GetActiveApplication());
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
#endif

  return env.Null();
}

Napi::Value SleepMilliseconds(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
#if defined(__APPLE__) || defined(__linux__)
  usleep(1000 * info[0].As<Napi::Number>().Int32Value());
#else
  Sleep(info[0].As<Napi::Number>().Int32Value());
#endif
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
  exports.Set(Napi::String::New(env, "focusApplication"),
              Napi::Function::New(env, FocusApplication));
  exports.Set(Napi::String::New(env, "getActiveApplication"),
              Napi::Function::New(env, GetActiveApplication));
  exports.Set(Napi::String::New(env, "getRunningApplications"),
              Napi::Function::New(env, GetRunningApplications));
  exports.Set(Napi::String::New(env, "pressKey"),
              Napi::Function::New(env, PressKey));
  exports.Set(Napi::String::New(env, "sleep"),
              Napi::Function::New(env, SleepMilliseconds));
  exports.Set(Napi::String::New(env, "typeText"),
              Napi::Function::New(env, TypeText));
  return exports;
}

NODE_API_MODULE(addon, Init)
