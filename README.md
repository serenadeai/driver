![Serenade Logo](https://cdn.serenade.ai/img/logo-small.png)

# serenade-driver

A collection of utilities for implementing cross-platform low-level system automations. Check out the `test` directory for example usages.

This module is used Serenade, the most powerful way to write code using natural speech. For more, see https://serenade.ai.

## Documentation

### click([button][, count])

Trigger a mouse click.

* `button <string>` The button to click. Can be `left`, `right`, or `middle`.
* `count <number>` How many times to click. For instance, `2` would be a double-click, and `3` would be a triple-click.

### clickButton(button)

Click a native system button matching the given text. Currently macOS only.

* `button <string>` Button to click. This value is a substring of the text displayed in the button.

### focusApplication(application)

Bring an application to the foreground.

* `application <string>` Application to focus. This value is a substring of the application's path.

### getActiveApplication

Get the path of the currently-active application.

* Returns: `<string>` path of the active application.

### getClickableButtons

Get a list of all of the buttons that can currently be clicked (i.e., are visible in the active application). Currently macOS only.

* Returns: `<string[]>` A list of button titles.

### getEditorState

Get the text and cursor position of the currently-active text field. Currently macOS only.

* Returns: `<{ text: string, cursor: number }>` Text field data.

### getInstalledApplications

Get a list of applications installed on the system.

* Returns: `<string[]>` List of application paths.

### getRunningApplications

Get a list of currently-running applications.

* Returns: `<string[]>` List of application paths.

### launchApplication(application)

Launch an application.

* `application <string>` Substring of the application to launch.

### pressKey(key[, modifiers][, count])

Press a key on the keyboard, optionally while holding down other keys.

* `key <string>` Key to press. Can be a letter, number, or the name of the key, like `enter`, `backspace`, or `comma`.
* `modifiers <string[]>` List of modifier keys to hold down while pressing the key. Can be one or more of `control`, `alt`, `command`, `option`, `shift`, or `function`.
* `count <number>` The number of times to press the key.

### quitApplication(application)

Quit an application.

* `application <string>` Substring of the application to quit.

### runShell(command[, args][, options][, callback])

Run a command at the shell.

* `command <string>` Name of the executable to run.
* `args <string[]>` List of arguments to pass to the executable.
* `options <object>` Object of spawn arguments. Can simply be `{}`. See https://nodejs.org/api/child_process.html#child_process_child_process_spawn_command_args_options for more.

### setEditorState(text, cursor)

Set the text and cursor position of the currently-active editor. Currently macOS only.

* `text <string>` New editor content.
* `cursor <number>` New editor cursor position.

### setMouseLocation(x, y)

Move the mouse to the given coordinates, with the origin at the top-left of the screen.

* `x <number>` x-coordinate of the mouse.
* `y <number>` y-coordinate of the mouse.

### typeText(text)

Type a string of text.

* `text <string>` Text to type.
