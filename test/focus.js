const driver = require("../index");

const run = async () => {
  await driver.focusApplication(process.argv[2]);
};

run();
