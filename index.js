const child_process = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");
const shortcut = require("windows-shortcuts");
const lib = require("bindings")("serenade-driver.node");

const applicationMatches = (application, possible, aliases) => {
  let alias = application.toLowerCase();
  if (aliases && aliases[alias]) {
    alias = aliases[alias];
  }

  return possible.filter(
    (e) =>
      e.toLowerCase().includes(application.toLowerCase()) ||
      e.toLowerCase().includes(application.toLowerCase().replace(/\s/g, "")) ||
      e.toLowerCase().includes(alias)
  );
};

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

exports.focusApplication = (application, aliases) => {
  const matching = applicationMatches(application, exports.getRunningApplications(), aliases);
  if (matching.length == 0) {
    return;
  }

  lib.focusApplication(matching[0]);
};

exports.focusOrLaunchApplication = (application, aliases) => {
  const matching = applicationMatches(application, exports.getRunningApplications(), aliases);
  if (matching.length == 0) {
    exports.launchApplication(application, aliases);
  } else {
    lib.focusApplication(matching[0]);
  }
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

exports.getInstalledApplications = () => {
  const search = (root, depth, max) => {
    let result = [];
    if (depth == max) {
      return result;
    }

    const files = fs.readdirSync(root, { withFileTypes: true });
    for (let e of files) {
      const file = path.join(root, e.name);
      if (os.platform() == "darwin" && file.endsWith(".app")) {
        result.push(file);
      } else if (os.platform() == "win32" && file.endsWith(".lnk")) {
        result.push(file);
      } else if (e.isDirectory()) {
        result = result.concat(search(file, depth + 1, max));
      }
    }

    return result;
  };

  const max = 2;
  if (os.platform() == "darwin") {
    return search("/Applications", 0, max).concat(search("/System/Applications", 0, max));
  } else if (os.platform() == "win32") {
    return search(path.join(os.homedir(), "Desktop"), 0, max).concat(
      search(path.join(process.env.APPDATA, "Microsoft", "Windows", "Start Menu", "Programs"), 0, max)).concat(
      search("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs", 0, max));
  }

  return [];
};

exports.getMouseLocation = () => {
  return lib.getMouseLocation();
};

exports.getRunningApplications = () => {
  return lib.getRunningApplications();
};

exports.launchApplication = (application, aliases) => {
  if (os.platform() == "linux") {
    child_process.spawn(application, [], { detached: true });
    return;
  }

  const matching = applicationMatches(application, exports.getInstalledApplications(), aliases);
  if (matching.length == 0) {
    return;
  }

  if (os.platform() == "darwin") {
    child_process.spawn("open", [matching[0]], { detached: true });
  } else if (os.platform() == "win32") {
    let app = matching[0];
    if (app.endsWith(".lnk")) {
      shortcut.query(app, (error, data) => {
        if (error) {
          return;
        }

        let args = [];
        if (data.expanded.args) {
          args = [data.expanded.args.replace(/"/g, "")];
        }

        let options = { detached: true };
        if (data.expanded.workingDir) {
          options.cwd = data.expanded.workingDir;
        }

        child_process.spawn(data.expanded.target, args, options);
      });
    } else {
      child_process.spawn(app, [], { detached: true });
    }
  }
};

exports.mouseDown = (button) => {
  if (!button) {
    button = "left";
  }

  lib.mouseDown(button);
};

exports.mouseUp = (button) => {
  if (!button) {
    button = "left";
  }

  lib.mouseUp(button);
};

exports.pressKey = (key, modifiers, count) => {
  if (!modifiers) {
    modifiers = [];
  }

  if (!count) {
    count = 1;
  }

  for (let i = 0; i < count; i++) {
    lib.pressKey(key, modifiers);
  }
};

exports.quitApplication = (application, aliases) => {
  if (applicationMatches(application, exports.getRunningApplications(), aliases).length == 0) {
    return;
  }

  let modifiers = ["alt"];
  let key = "f4";
  if (os.platform() == "darwin") {
    modifiers = ["command"];
    key = "q";
  }

  lib.focusApplication(application);
  lib.pressKey(key, modifiers);
};

exports.runShell = (command, args, options, callback) => {
  let stdout = "";
  let stderr = "";
  if (!options) {
    options = {};
  }

  const spawned = child_process.spawn(command, args, options);

  spawned.stdout.on("data", (data) => {
    stdout += data;
  });

  spawned.stderr.on("data", (data) => {
    stderr += data;
  });

  spawned.on("close", () => {
    if (callback) {
      callback({ stdout, stderr });
    }
  });
};

exports.setEditorState = (text, cursor, cursorEnd) => {
  if (!cursorEnd) {
    cursorEnd = 0;
  }

  lib.setEditorState(text, cursor, cursorEnd);
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

exports.typeText = (text) => {
  if (text) {
    const lines = text.split(/\n/);
    if (lines.length) {
      for (const line of lines.slice(0, lines.length - 1)) {
        lib.typeText(line);
        lib.pressKey("enter");
      }

      lib.typeText(lines.pop());
    }
  }
};
