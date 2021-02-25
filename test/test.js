const driver = require("../index");

const run = async () => {
  console.log("Active application:", driver.getActiveApplication());
  console.log("Running applications:", driver.getRunningApplications().slice(0, 5));
  console.log("Installed applications:", (await driver.getInstalledApplications()).slice(0, 5));

  console.log("Double clicking");
  driver.click("left", 2);

  console.log("Setting mouse location");
  driver.setMouseLocation(200, 200);

  console.log("Running a command");
  driver.runShell("ls", ["-lah"], {}, (data) => {
    console.log(data);
  });

  console.log('Typing "My password is Password123!"');
  driver.typeText("My password is Password123!");

  console.log("Pressing a");
  driver.pressKey("a");

  if (process.platform == "darwin") {
    console.log("Pressing command+tab");
    driver.pressKey("tab", ["command"]);
    console.log("Launching calculator...");
    driver.launchApplication("calc");

    setTimeout(() => {
      console.log("Quitting calculator...");
      driver.quitApplication("calc");
    }, 1000);
  } else {
    console.log("Pressing alt+tab");
    driver.pressKey("tab", ["alt"]);
  }
};

run();
