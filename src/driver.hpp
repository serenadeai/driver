#include <napi.h>

#include <vector>

Napi::Value Click(const Napi::CallbackInfo& info);
Napi::Value ClickButton(const Napi::CallbackInfo& info);
Napi::Value FocusApplication(const Napi::CallbackInfo& info);
Napi::Value GetActiveApplication(const Napi::CallbackInfo& info);
Napi::Value GetClickableButtons(const Napi::CallbackInfo& info);
Napi::Value GetEditorState(const Napi::CallbackInfo& info);
Napi::Array GetRunningApplications(const Napi::CallbackInfo& info);
Napi::Value PressKey(const Napi::CallbackInfo& info);
Napi::Value SetEditorState(const Napi::CallbackInfo& info);
Napi::Value SetMouseLocation(const Napi::CallbackInfo& info);
Napi::Value TypeText(const Napi::CallbackInfo& info);
Napi::Object Init(Napi::Env env, Napi::Object exports);
