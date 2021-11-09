const driver = require("../index");

setInterval(async () => {
  console.log(await driver.getRunningApplications());
}, 500);
