const driver = require("../index");

const run = async () => {
  console.log("Active application:", await driver.getActiveApplication());
  console.log("Running applications:", (await driver.getRunningApplications()).slice(0, 5));
  console.log("Installed applications:", (await driver.getInstalledApplications()).slice(0, 5));
  console.log("Mouse location:", await driver.getMouseLocation());

  console.log("Running a command");
  console.log(await driver.runShell("ls", ["-lah"]));

  console.log('Typing "My password is Password123!\\n"');
  await driver.typeText("My password is Password123!\n");

  console.log("Pressing backspace 14 times");
  await driver.pressKey("backspace", [], 14);

  console.log('Typing "a\\nb\\n"');
  await driver.typeText("a\nb\n");

  console.log("Pressing c");
  await driver.pressKey("c");

  console.log("NOT pressing backspace");
  await driver.pressKey("backspace", [], 0);

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

console.log("Sleeping for 3 seconds so you can focus another app like TextEdit ...");
setTimeout(() => run(), 3000);
