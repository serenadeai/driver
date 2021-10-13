const driver = require("../index");

const run = async () => {
  setInterval(async () => {
    const mouseLocation = driver.getMouseLocation();
    console.log(mouseLocation);
  }, 500);
  setInterval(async () => {
    driver.click();
  }, 2000)
};

run();