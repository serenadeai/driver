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

  if (count === undefined || count === false) {
    count = 1;
  }

  if (count < 1) {
    return;
  }

  return lib.click(button, count);
};

exports.clickButton = (button, count) => {
  if (count === undefined || count === false) {
    count = 1;
  }

  if (count < 1) {
    return;
  }

  return lib.clickButton(button, count);
};

exports.delay = (timeout) => {
  return new Promise((resolve) => {
    setTimeout(() => {
      resolve();
    }, timeout);
  });
};

exports.focusApplication = async (application, aliases) => {
  const matching = applicationMatches(application, await exports.getRunningApplications(), aliases);
  if (matching.length == 0) {
    return;
  }

  return lib.focusApplication(matching[0]);
};

exports.focusOrLaunchApplication = async (application, aliases) => {
  const matching = applicationMatches(application, await exports.getRunningApplications(), aliases);
  if (matching.length == 0) {
    return exports.launchApplication(application, aliases);
  } else {
    return lib.focusApplication(matching[0]);
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

exports.getEditorStateFallback = () => {
  return lib.getEditorStateFallback();
};

exports.getInstalledApplications = async () => {
  const search = async (root, depth, max) => {
    let result = [];
    if (depth == max) {
      return Promise.resolve(result);
    }

    return new Promise(async (resolve) => {
      fs.readdir(root, { withFileTypes: true }, async (error, files) => {
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

        resolve(result);
      });
    });
  };

  const max = 2;
  if (os.platform() == "darwin") {
    return (await search("/Applications", 0, max)).concat(
      await search("/System/Applications", 0, max)
    );
  } else if (os.platform() == "win32") {
    return (await search(path.join(os.homedir(), "Desktop"), 0, max))
      .concat(
        await search(
          path.join(process.env.APPDATA, "Microsoft", "Windows", "Start Menu", "Programs"),
          0,
          max
        )
      )
      .concat(await search("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs", 0, max));
  }

  return Promise.resolve([]);
};

exports.getMouseLocation = () => {
  return lib.getMouseLocation();
};

exports.getRunningApplications = () => {
  return lib.getRunningApplications();
};

exports.launchApplication = async (application, aliases) => {
  if (os.platform() == "linux") {
    child_process.spawn(application, [], { detached: true });
    return;
  }

  const matching = applicationMatches(
    application,
    await exports.getInstalledApplications(),
    aliases
  );

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

  return lib.mouseDown(button);
};

exports.mouseUp = (button) => {
  if (!button) {
    button = "left";
  }

  return lib.mouseUp(button);
};

exports.pressKey = (key, modifiers, count) => {
  if (!modifiers) {
    modifiers = [];
  }

  if (count === undefined || count === false) {
    count = 1;
  }

  if (count < 1) {
    return;
  }

  return lib.pressKey(key, modifiers, count);
};

exports.quitApplication = async (application, aliases) => {
  if (
    applicationMatches(application, await exports.getRunningApplications(), aliases).length == 0
  ) {
    return;
  }

  let modifiers = ["alt"];
  let key = "f4";
  if (os.platform() == "darwin") {
    modifiers = ["command"];
    key = "q";
  }

  await lib.focusApplication(application);
  return lib.pressKey(key, modifiers, 1);
};

exports.runShell = async (command, args, options) => {
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

  return new Promise((resolve) => {
    spawned.on("close", () => {
      resolve({ stdout, stderr });
    });
  });
};

exports.setEditorState = (text, cursor, cursorEnd) => {
  if (!cursorEnd) {
    cursorEnd = 0;
  }

  return lib.setEditorState(text, cursor, cursorEnd);
};

exports.setMouseLocation = (x, y) => {
  return lib.setMouseLocation(x, y);
};

exports.typeText = (text) => {
  if (!text) {
    return;
  }

  return lib.typeText(text);
};
