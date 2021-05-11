#include <napi.h>

#include <vector>

Napi::Promise Click(const Napi::CallbackInfo& info);
Napi::Promise ClickButton(const Napi::CallbackInfo& info);
Napi::Promise FocusApplication(const Napi::CallbackInfo& info);
Napi::Promise GetActiveApplication(const Napi::CallbackInfo& info);
Napi::Promise GetClickableButtons(const Napi::CallbackInfo& info);
Napi::Promise GetEditorState(const Napi::CallbackInfo& info);
Napi::Promise GetEditorStateFallback(const Napi::CallbackInfo& info);
Napi::Promise GetMouseLocation(const Napi::CallbackInfo& info);
Napi::Promise GetRunningApplications(const Napi::CallbackInfo& info);
Napi::Promise MouseDown(const Napi::CallbackInfo& info);
Napi::Promise MouseUp(const Napi::CallbackInfo& info);
Napi::Promise PressKey(const Napi::CallbackInfo& info);
Napi::Promise Select(const Napi::CallbackInfo& info);
Napi::Promise SetEditorState(const Napi::CallbackInfo& info);
Napi::Promise SetMouseLocation(const Napi::CallbackInfo& info);
Napi::Promise TypeText(const Napi::CallbackInfo& info);

Napi::Object Init(Napi::Env env, Napi::Object exports);
