<img src="https://cdn.serenade.ai/img/logo-github.png" width="250px" alt="Serenade Logo" />

# serenade-driver

A collection of utilities for implementing cross-platform low-level system automations. Check out the `test` directory for example usages.

This module is used by Serenade, the most powerful way to write code using natural speech. For more, see https://serenade.ai.

## Documentation

All functions in this module return `Promise` objects that will be fulfilled when system calls complete. So, you can either `await` each function, or use `then` to attach a callback when they complete.

### click([button][, count])

Trigger a mouse click.

* `button <string>` Mouse button to click. Can be `left`, `right`, or `middle`.
* `count <number>` How many times to click. For instance, `2` would be a double-click, and `3` would be a triple-click.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### clickButton(button)

Click a native system button matching the given text. Currently macOS only.

* `button <string>` Button to click. This value is a substring of the text displayed in the button.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### focusApplication(application)

Bring an application to the foreground.

* `application <string>` Application to focus. This value is a substring of the application's path.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### getActiveApplication()

Get the path of the currently-active application.

* Returns `<Promise<string>>` Fulfills with the name of the active application upon success.

### getClickableButtons()

Get a list of all of the buttons that can currently be clicked (i.e., are visible in the active application). Currently macOS only.

* Returns: `<Promise<string[]>>` Fulfills with a list of button titles upon success.

### getEditorState()

Get the text and cursor position of the currently-active text field. Currently macOS only.

* Returns: `<Promise<{ text: string, cursor: number }>>` Fulfills with text field data upon success.

### getInstalledApplications()

Get a list of applications installed on the system.

* Returns: `<Promise<string[]>>` Fulfills with a list of application paths upon success.

### getMouseLocation()

Get the current (x, y) coordinates of the mouse.

* Returns: `<Promise<{ x: number, y: number }>>` Fulfills with the location of the mouse upon success.

### getRunningApplications()

Get a list of currently-running applications.

* Returns: `<Promise<string[]>>` Fulfills with a list of application paths upon success.

### launchApplication(application)

Launch an application.

* `application <string>` Substring of the application to launch.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### mouseDown(button)

Press the mouse down.

* `button <string>` Mouse button to press. Can be `left`, `middle`, or `right`.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### mouseUp(button)

Release a mouse press.

* `button <string>` Mouse button to release. Can be `left`, `middle`, or `right`.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### pressKey(key[, modifiers][, count])

Press a key on the keyboard, optionally while holding down other keys.

* `key <string>` Key to press. Can be a letter, number, or the name of the key, like `enter`, `backspace`, or `comma`.
* `modifiers <string[]>` List of modifier keys to hold down while pressing the key. Can be one or more of `control`, `alt`, `command`, `option`, `shift`, or `function`.
* `count <number>` The number of times to press the key.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### quitApplication(application)

Quit an application.

* `application <string>` Substring of the application to quit.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### runShell(command[, args][, options][, callback])

Run a command at the shell.

* `command <string>` Name of the executable to run.
* `args <string[]>` List of arguments to pass to the executable.
* `options <object>` Object of spawn arguments. Can simply be `{}`. See https://nodejs.org/api/child_process.html#child_process_child_process_spawn_command_args_options for more.
* Returns `<Promise<{ stdout: string, stderr: string }>>` Fulfills with the output of the command upon success.

### setEditorState(text, cursor)

Set the text and cursor position of the currently-active editor. Currently macOS only.

* `text <string>` New editor content.
* `cursor <number>` New editor cursor position.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### setMouseLocation(x, y)

Move the mouse to the given coordinates, with the origin at the top-left of the screen.

* `x <number>` x-coordinate of the mouse.
* `y <number>` y-coordinate of the mouse.
* Returns `<Promise>` Fulfills with `undefined` upon success.

### typeText(text)

Type a string of text.

* `text <string>` Text to type.
* Returns `<Promise>` Fulfills with `undefined` upon success.

## Building & Testing

To build the library, simply run:

    yarn

Then, to test, you can run:

    node test/test.js

That test file will simulate a bunch of keystrokes, so if your computer looks like it's going crazy, don't worry. Probably.
