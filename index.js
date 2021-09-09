const child_process = require("child_process");
const exp = require("constants");
const fs = require("fs");
const os = require("os");
const path = require("path");
const shortcut = require("windows-shortcuts");
const lib = require("bindings")("serenade-driver.node");

const applicationMatches = (application, possible, aliases) => {
  let alias = application;
  if (aliases && aliases[application]) {
    alias = normalizeApplication(aliases[application]);
  }

  return possible.filter((e) => {
    const possibility = normalizeApplication(e);
    return possibility.includes(application) || possibility.includes(alias);
  });
};

const normalizeApplication = (s) => {
  return s.toLowerCase().replace(/ /g, "");
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
  application = normalizeApplication(application);

  // if we have an exact match without any aliasing, then prioritize that
  if (
    applicationMatches(application, await exports.getRunningApplications(), {})
      .length > 0
  ) {
    return lib.focusApplication(application);
  }

  // otherwise, try to focus using the alias map
  if (aliases && aliases[application]) {
    application = normalizeApplication(aliases[application]);
  }

  return lib.focusApplication(application);
};

exports.focusOrLaunchApplication = async (application, aliases) => {
  application = normalizeApplication(application);
  const running = await exports.getRunningApplications();

  // if we have an exact match or an aliased match, then we want to focus instead of launching
  const matching =
    applicationMatches(application, running, aliases).length > 0 ||
    applicationMatches(application, running, {}).length > 0;

  if (matching.length == 0) {
    return exports.launchApplication(application, aliases);
  } else {
    return exports.focusApplication(application, aliases);
  }
};

exports.getActiveApplication = () => {
  return lib.getActiveApplication();
};

exports.getActiveApplicationWindowBounds = () => {
  return lib.getActiveApplicationWindowBounds();
};

exports.getClickableButtons = () => {
  return lib.getClickableButtons();
};

exports.getEditorState = () => {
  return lib.getEditorState();
};

exports.getEditorStateFallback = (paragraph) => {
  return lib.getEditorStateFallback(!!paragraph);
};

exports.getInstalledApplications = async () => {
  const search = async (root, depth, max) => {
    let result = [];
    if (depth == max) {
      return Promise.resolve(result);
    }

    return new Promise(async (resolve) => {
      fs.readdir(root, { withFileTypes: true }, async (error, files) => {
        if (!error && files && files.length) {
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
          path.join(
            process.env.APPDATA,
            "Microsoft",
            "Windows",
            "Start Menu",
            "Programs"
          ),
          0,
          max
        )
      )
      .concat(
        await search(
          "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs",
          0,
          max
        )
      );
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

  application = normalizeApplication(application);
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

        child_process.spawn(path.basename(data.expanded.target), args, options);
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
  if (!application) {
    return;
  }

  if (
    applicationMatches(
      application,
      await exports.getRunningApplications(),
      aliases
    ).length == 0
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
  await exports.delay(100);
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
