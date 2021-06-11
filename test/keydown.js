const driver = require("../index");

const run = async () => {
  const modifier = process.platform == "darwin" ? "command" : "alt"
  await driver.keyDown(modifier);
  await driver.pressKey("tab");
  await driver.delay(1000);
  await driver.pressKey("tab");
  await driver.keyUp(modifier);
};

run();