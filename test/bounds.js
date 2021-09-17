const driver = require("../index");

const run = async () => {
  setInterval(async() => {
    const bounds = await driver.getActiveApplicationWindowBounds();
    console.log(bounds);
  }, 500);
}

run();
