#include <napi.h>

#include <tuple>
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

ClickAsyncWorker::ClickAsyncWorker(std::string button, int count,
                                   const Napi::Function& callback)
    : Napi::AsyncWorker(callback), button_(button), count_(count) {}

void ClickAsyncWorker::Execute() {
#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
  driver::Click(display, button_, count_);
  XCloseDisplay(display);
#else
  driver::Click(button_, count_);
#endif
}

void ClickAsyncWorker::OnOK() { Callback().Call({}); }

void Click(const Napi::CallbackInfo& info) {
  ClickAsyncWorker* worker = new ClickAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(),
      info[1].As<Napi::Number>().Int32Value(), info[2].As<Napi::Function>());
  worker->Queue();
}

ClickButtonAsyncWorker::ClickButtonAsyncWorker(std::string button, int count,
                                               const Napi::Function& callback)
    : Napi::AsyncWorker(callback), button_(button), count_(count) {}

void ClickButtonAsyncWorker::Execute() { driver::ClickButton(button_, count_); }

void ClickButtonAsyncWorker::OnOK() { Callback().Call({}); }

void ClickButton(const Napi::CallbackInfo& info) {
  ClickButtonAsyncWorker* worker = new ClickButtonAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(),
      info[1].As<Napi::Number>().Int32Value(), info[2].As<Napi::Function>());
  worker->Queue();
}

FocusApplicationAsyncWorker::FocusApplicationAsyncWorker(
    std::string application, const Napi::Function& callback)
    : Napi::AsyncWorker(callback), application_(application) {}

void FocusApplicationAsyncWorker::Execute() {
  driver::FocusApplication(application_);
}

void FocusApplicationAsyncWorker::OnOK() { Callback().Call({}); }

void FocusApplication(const Napi::CallbackInfo& info) {
  FocusApplicationAsyncWorker* worker = new FocusApplicationAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(), info[1].As<Napi::Function>());
  worker->Queue();
}

GetActiveApplicationAsyncWorker::GetActiveApplicationAsyncWorker(
    const Napi::Function& callback)
    : Napi::AsyncWorker(callback) {}

void GetActiveApplicationAsyncWorker::Execute() {
  active_ = driver::GetActiveApplication();
}

void GetActiveApplicationAsyncWorker::OnOK() {
  Callback().Call({Env().Undefined(), Napi::String::New(Env(), active_)});
}

void GetActiveApplication(const Napi::CallbackInfo& info) {
  GetActiveApplicationAsyncWorker* worker =
      new GetActiveApplicationAsyncWorker(info[0].As<Napi::Function>());
  worker->Queue();
}

GetClickableButtonsAsyncWorker::GetClickableButtonsAsyncWorker(
    const Napi::Function& callback)
    : Napi::AsyncWorker(callback) {}

void GetClickableButtonsAsyncWorker::Execute() {
  buttons_ = driver::GetClickableButtons();
}

void GetClickableButtonsAsyncWorker::OnOK() {
#ifdef __APPLE__
  Napi::Array result = Napi::Array::New(Env(), buttons_.size());
  for (size_t i = 0; i < buttons_.size(); i++) {
    result[i] = buttons_[i];
  }
#else
  Napi::Array result = Napi::Array::New(Env(), 0);
#endif

  Callback().Call({Env().Undefined(), result});
}

void GetClickableButtons(const Napi::CallbackInfo& info) {
  GetClickableButtonsAsyncWorker* worker =
      new GetClickableButtonsAsyncWorker(info[0].As<Napi::Function>());
  worker->Queue();
}

GetEditorStateAsyncWorker::GetEditorStateAsyncWorker(
    const Napi::Function& callback)
    : Napi::AsyncWorker(callback) {}

void GetEditorStateAsyncWorker::Execute() {
  text_ = driver::GetEditorSource();
  cursor_ = driver::GetEditorCursor();
}

void GetEditorStateAsyncWorker::OnOK() {
  Napi::Object result = Napi::Object::New(Env());

#ifdef __APPLE__
  result.Set("text", text_);
  result.Set("cursor", cursor_);
#endif

  Callback().Call({Env().Undefined(), result});
}

void GetEditorState(const Napi::CallbackInfo& info) {
  GetEditorStateAsyncWorker* worker =
      new GetEditorStateAsyncWorker(info[0].As<Napi::Function>());
  worker->Queue();
}

GetMouseLocationAsyncWorker::GetMouseLocationAsyncWorker(
    const Napi::Function& callback)
    : Napi::AsyncWorker(callback) {}

void GetMouseLocationAsyncWorker::Execute() {
  std::tuple<int, int> location = driver::GetMouseLocation();
  x_ = std::get<0>(location);
  y_ = std::get<1>(location);
}

void GetMouseLocationAsyncWorker::OnOK() {
  Napi::Object result = Napi::Object::New(Env());
  result.Set("x", x_);
  result.Set("y", y_);
  Callback().Call({Env().Undefined(), result});
}

void GetMouseLocation(const Napi::CallbackInfo& info) {
  GetMouseLocationAsyncWorker* worker =
      new GetMouseLocationAsyncWorker(info[0].As<Napi::Function>());
  worker->Queue();
}

GetRunningApplicationsAsyncWorker::GetRunningApplicationsAsyncWorker(
    const Napi::Function& callback)
    : Napi::AsyncWorker(callback) {}

void GetRunningApplicationsAsyncWorker::Execute() {
  running_ = driver::GetRunningApplications();
}

void GetRunningApplicationsAsyncWorker::OnOK() {
  Napi::Array result = Napi::Array::New(Env(), running_.size());
  for (size_t i = 0; i < running_.size(); i++) {
    result[i] = running_[i];
  }

  Callback().Call({Env().Undefined(), result});
}

void GetRunningApplications(const Napi::CallbackInfo& info) {
  GetRunningApplicationsAsyncWorker* worker =
      new GetRunningApplicationsAsyncWorker(info[0].As<Napi::Function>());
  worker->Queue();
}

MouseDownAsyncWorker::MouseDownAsyncWorker(std::string button,
                                           const Napi::Function& callback)
    : Napi::AsyncWorker(callback), button_(button) {}

void MouseDownAsyncWorker::Execute() {
#if __linux__
  Display* display = XOpenDisplay(NULL);
  driver::MouseDown(display, button_);
  XCloseDisplay(display);
#else
  driver::MouseDown(button_);
#endif
}

void MouseDownAsyncWorker::OnOK() { Callback().Call({}); }

void MouseDown(const Napi::CallbackInfo& info) {
  MouseDownAsyncWorker* worker = new MouseDownAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(), info[1].As<Napi::Function>());
  worker->Queue();
}

MouseUpAsyncWorker::MouseUpAsyncWorker(std::string button,
                                       const Napi::Function& callback)
    : Napi::AsyncWorker(callback), button_(button) {}

void MouseUpAsyncWorker::Execute() {
#if __linux__
  Display* display = XOpenDisplay(NULL);
  driver::MouseUp(display, button_);
  XCloseDisplay(display);
#else
  driver::MouseUp(button_);
#endif
}

void MouseUpAsyncWorker::OnOK() { Callback().Call({}); }

void MouseUp(const Napi::CallbackInfo& info) {
  MouseUpAsyncWorker* worker = new MouseUpAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(), info[1].As<Napi::Function>());
  worker->Queue();
}

PressKeyAsyncWorker::PressKeyAsyncWorker(std::string key,
                                         std::vector<std::string> modifiers,
                                         int count,
                                         const Napi::Function& callback)
    : Napi::AsyncWorker(callback),
      key_(key),
      modifiers_(modifiers),
      count_(count) {}

void PressKeyAsyncWorker::Execute() {
#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  for (int i = 0; i < count_; i++) {
#ifdef __linux__
    driver::PressKey(display, key_, modifiers_);
    usleep(100000);
#else
    driver::PressKey(key_, modifiers_);
#endif
  }

#ifdef __linux__
  XCloseDisplay(display);
#endif
}

void PressKeyAsyncWorker::OnOK() { Callback().Call({}); }

void PressKey(const Napi::CallbackInfo& info) {
  std::vector<std::string> modifiers;
  Napi::Array array = info[1].As<Napi::Array>();
  for (uint32_t i = 0; i < array.Length(); i++) {
    Napi::Value e = array[i];
    modifiers.push_back(e.As<Napi::String>().Utf8Value());
  }

  PressKeyAsyncWorker* worker = new PressKeyAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(), modifiers,
      info[2].As<Napi::Number>(), info[3].As<Napi::Function>());
  worker->Queue();
}

SetEditorStateAsyncWorker::SetEditorStateAsyncWorker(
    std::string text, int cursor, int cursorEnd, const Napi::Function& callback)
    : Napi::AsyncWorker(callback),
      text_(text),
      cursor_(cursor),
      cursorEnd_(cursorEnd) {}

void SetEditorStateAsyncWorker::Execute() {
#ifdef __APPLE__
  driver::SetEditorState(text_, cursor_, cursorEnd_);
#endif
}

void SetEditorStateAsyncWorker::OnOK() { Callback().Call({}); }

void SetEditorState(const Napi::CallbackInfo& info) {
  SetEditorStateAsyncWorker* worker = new SetEditorStateAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(),
      info[1].As<Napi::Number>().Int32Value(),
      info[2].As<Napi::Number>().Int32Value(), info[3].As<Napi::Function>());
  worker->Queue();
}

SetMouseLocationAsyncWorker::SetMouseLocationAsyncWorker(
    const int x, const int y, const Napi::Function& callback)
    : Napi::AsyncWorker(callback), x_(x), y_(y) {}

void SetMouseLocationAsyncWorker::Execute() {
  driver::SetMouseLocation(x_, y_);
}

void SetMouseLocationAsyncWorker::OnOK() { Callback().Call({}); }

void SetMouseLocation(const Napi::CallbackInfo& info) {
  SetMouseLocationAsyncWorker* worker = new SetMouseLocationAsyncWorker(
      info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>(),
      info[2].As<Napi::Function>());
  worker->Queue();
}

TypeTextAsyncWorker::TypeTextAsyncWorker(std::string text,
                                         const Napi::Function& callback)
    : Napi::AsyncWorker(callback), text_(text) {}

void TypeTextAsyncWorker::Execute() {
#ifdef __linux__
  Display* display = XOpenDisplay(NULL);
#endif

  std::vector<std::string> modifiers;
  for (char c : text_) {
#ifdef __linux__
    driver::PressKey(display, std::string(1, c), modifiers);
#else
    driver::PressKey(std::string(1, c), modifiers);
#endif
  }

#ifdef __linux__
  XCloseDisplay(display);
#endif
}

void TypeTextAsyncWorker::OnOK() { Callback().Call({}); }

void TypeText(const Napi::CallbackInfo& info) {
  TypeTextAsyncWorker* worker = new TypeTextAsyncWorker(
      info[0].As<Napi::String>().Utf8Value(), info[1].As<Napi::Function>());
  worker->Queue();
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
  exports.Set(Napi::String::New(env, "getMouseLocation"),
              Napi::Function::New(env, GetMouseLocation));
  exports.Set(Napi::String::New(env, "getRunningApplications"),
              Napi::Function::New(env, GetRunningApplications));
  exports.Set(Napi::String::New(env, "pressKey"),
              Napi::Function::New(env, PressKey));
  exports.Set(Napi::String::New(env, "mouseDown"),
              Napi::Function::New(env, MouseDown));
  exports.Set(Napi::String::New(env, "mouseUp"),
              Napi::Function::New(env, MouseUp));
  exports.Set(Napi::String::New(env, "setEditorState"),
              Napi::Function::New(env, SetEditorState));
  exports.Set(Napi::String::New(env, "setMouseLocation"),
              Napi::Function::New(env, SetMouseLocation));
  exports.Set(Napi::String::New(env, "typeText"),
              Napi::Function::New(env, TypeText));

  return exports;
}

NODE_API_MODULE(addon, Init)
