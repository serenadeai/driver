const driver = require("../index");

const run = async () => {
  await driver.delay(1000);
  const bounds = await driver.getActiveApplicationWindowBounds();
  console.log(bounds);
};

run();
