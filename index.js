const fs = require("fs");
const os = require("os");
const path = require("path");
const shortcut = require("windows-shortcuts");
const lib = require("bindings")("driver.node");

exports.click = (button, count) => {
  if (!button) {
    button = "left";
  }

  if (!count) {
    count = 1;
  }

  lib.click(button, count);
};

exports.clickButton = (button) => {
  lib.clickButton(button);
};

exports.doubleClick = (button) => {
  return exports.click(button, 2);
};

exports.focusApplication = (app) => {
  lib.focusApplication(app);
};

exports.getActiveApplication = () => {
  return lib.getActiveApplication();
};

exports.getClickableButtons = () => {
  return lib.getClickableButtons();
};

exports.getEditorState = () => {
  return lib.getEditorState();
};

exports.getInstalledApplications = async () => {
  const search = async (root, depth, max) => {
    let result = [];
    if (depth == max) {
      return result;
    }

    const files = await fs.promises.readdir(root, { withFileTypes: true });
    for (let e of files) {
      const file = path.join(root, e.name);
      if (os.platform() == "darwin" && file.endsWith(".app")) {
        result.push(file);
      } else if (os.platform() == "win32" && file.endsWith(".lnk")) {
        result.push(file);
      } else if (e.isDirectory()) {
        result = result.concat(await search(file, depth + 1, max));
      }
    }

    return result;
  };

  const max = 2;
  if (os.platform() == "darwin") {
    return (await search("/Applications", 0, max)).concat(
      await search("/System/Applications", 0, max)
    );
  } else if (os.platform() == "win32") {
    return (await search(app.getPath("desktop"), 0, max))
      .concat(
        await search(`${app.getPath("appData")}\\Microsoft\\Windows\\Start Menu\\Programs`, 0, max)
      )
      .concat(await search("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs", 0, max));
  }

  return [];
};

exports.getRunningApplications = () => {
  return lib.getRunningApplications();
};

exports.pressKey = (key, modifiers) => {
  if (!modifiers) {
    modifiers = [];
  }

  lib.pressKey(key, modifiers);
};

exports.setEditorState = (source, cursor, cursorEnd) => {
  if (!cursorEnd) {
    cursorEnd = 0;
  }

  lib.setEditorState(source, cursor, cursorEnd);
};

exports.setMouseLocation = (x, y) => {
  if (x < 0) {
    x = 0;
  }

  if (y < 0) {
    y = 0;
  }

  lib.setMouseLocation(x, y);
};

exports.sleep = (timeout) => {
  lib.sleep(timeout);
};

exports.typeText = (text) => {
  lib.typeText(text);
};
