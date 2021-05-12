const driver = require("../index");

setInterval(async () => {
  console.log(await driver.getActiveApplication());
}, 500);
