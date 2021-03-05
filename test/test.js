const driver = require("../index");

const run = async () => {
  console.log("Active application:", await driver.getActiveApplication());
  console.log("Running applications:", (await driver.getRunningApplications()).slice(0, 5));
  console.log("Installed applications:", (await driver.getInstalledApplications()).slice(0, 5));
  console.log("Mouse location:", await driver.getMouseLocation());

  console.log("Running a command");
  console.log(await driver.runShell("ls", ["-lah"]));

  console.log('Typing "My password is Password123!"');
  await driver.typeText("My password is Password123!");

  console.log("Pressing a");
  await driver.pressKey("a");

  if (process.platform == "darwin") {
    console.log("Pressing menu");
    await driver.setMouseLocation(140, 10);
    await driver.mouseDown();
    setTimeout(async () => {
      await driver.mouseUp();

      console.log("Pressing command+tab");
      await driver.pressKey("tab", ["command"]);
      console.log("Launching calculator...");
      await driver.launchApplication("calc");

      setTimeout(async () => {
        console.log("Quitting calculator...");
        await driver.quitApplication("calc");
      }, 1000);
    }, 1000);
  } else if (process.platform == "linux") {
    console.log("Pressing menu");
    await driver.setMouseLocation(80, 65);
    await driver.mouseDown();
    setTimeout(async () => {
      await driver.mouseUp();

      console.log("Pressing alt+tab");
      await driver.pressKey("tab", ["alt"]);

      console.log("Double clicking");
      await driver.click("left", 2);
    }, 1000);
  } else {
    console.log("Pressing menu");
    await driver.setMouseLocation(20, 20);
    await driver.mouseDown();
    setTimeout(async () => {
      await driver.mouseUp();

      setTimeout(async () => {
        console.log("Pressing alt+tab");
        await driver.pressKey("tab", ["alt"]);

        console.log("Double clicking");
        await driver.click("left", 2);
      }, 100);
    }, 1000);
  }
};

run();
