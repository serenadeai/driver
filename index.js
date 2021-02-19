const lib = require("bindings")("driver.node");

exports.focusApplication = (app) => {
  lib.focusApplication(app);
};

exports.getActiveApplication = () => {
  return lib.getActiveApplication();
};

exports.getRunningApplications = () => {
  return lib.getRunningApplications();
};

exports.pressKey = (key, modifiers) => {
  lib.pressKey(key, modifiers);
};

exports.sleep = (timeout) => {
  lib.sleep(timeout);
};

exports.typeText = (text) => {
  lib.typeText(text);
};
