#include <napi.h>

#include <vector>

class ClickAsyncWorker : public Napi::AsyncWorker {
 public:
  ClickAsyncWorker(std::string button, int count,
                   const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string button_;
  int count_;
};

class ClickButtonAsyncWorker : public Napi::AsyncWorker {
 public:
  ClickButtonAsyncWorker(std::string button, int count,
                         const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string button_;
  int count_;
};

class FocusApplicationAsyncWorker : public Napi::AsyncWorker {
 public:
  FocusApplicationAsyncWorker(std::string application,
                              const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string application_;
};

class GetActiveApplicationAsyncWorker : public Napi::AsyncWorker {
 public:
  GetActiveApplicationAsyncWorker(const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string active_;
};

class GetClickableButtonsAsyncWorker : public Napi::AsyncWorker {
 public:
  GetClickableButtonsAsyncWorker(const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::vector<std::string> buttons_;
};

class GetEditorStateAsyncWorker : public Napi::AsyncWorker {
 public:
  GetEditorStateAsyncWorker(const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string text_;
  int cursor_;
};

class GetMouseLocationAsyncWorker : public Napi::AsyncWorker {
 public:
  GetMouseLocationAsyncWorker(const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  int x_;
  int y_;
};

class GetRunningApplicationsAsyncWorker : public Napi::AsyncWorker {
 public:
  GetRunningApplicationsAsyncWorker(const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::vector<std::string> running_;
};

class MouseDownAsyncWorker : public Napi::AsyncWorker {
 public:
  MouseDownAsyncWorker(std::string button, const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string button_;
};

class MouseUpAsyncWorker : public Napi::AsyncWorker {
 public:
  MouseUpAsyncWorker(std::string button, const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string button_;
};

class PressKeyAsyncWorker : public Napi::AsyncWorker {
 public:
  PressKeyAsyncWorker(std::string key, std::vector<std::string> modifiers,
                      int count, const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string key_;
  std::vector<std::string> modifiers_;
  int count_;
};

class SetEditorStateAsyncWorker : public Napi::AsyncWorker {
 public:
  SetEditorStateAsyncWorker(std::string text, int cursor, int cursorEnd,
                            const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string text_;
  int cursor_;
  int cursorEnd_;
};

class SetMouseLocationAsyncWorker : public Napi::AsyncWorker {
 public:
  SetMouseLocationAsyncWorker(int x, int y, const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  int x_;
  int y_;
};

class TypeTextAsyncWorker : public Napi::AsyncWorker {
 public:
  TypeTextAsyncWorker(std::string text, const Napi::Function& callback);
  void Execute();
  void OnOK();

 private:
  std::string text_;
};

void Click(const Napi::CallbackInfo& info);
void ClickButton(const Napi::CallbackInfo& info);
void FocusApplication(const Napi::CallbackInfo& info);
void GetActiveApplication(const Napi::CallbackInfo& info);
void GetClickableButtons(const Napi::CallbackInfo& info);
void GetEditorState(const Napi::CallbackInfo& info);
void GetMouseLocation(const Napi::CallbackInfo& info);
void GetRunningApplications(const Napi::CallbackInfo& info);
void MouseDown(const Napi::CallbackInfo& info);
void MouseUp(const Napi::CallbackInfo& info);
void PressKey(const Napi::CallbackInfo& info);
void SetEditorState(const Napi::CallbackInfo& info);
void SetMouseLocation(const Napi::CallbackInfo& info);
void TypeText(const Napi::CallbackInfo& info);

Napi::Object Init(Napi::Env env, Napi::Object exports);
