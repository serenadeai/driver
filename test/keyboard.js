const driver = require("../index");

const run = async () => {
  const text = "This is a sentence.";
  for (let i = 0; i < 25; i++) {
    await driver.typeText(text);
  }

  for (let i = 0; i < 25; i++) {
    await driver.typeText(text);
    await driver.pressKey("backspace", [], text.length);
  }
};

console.log("Sleeping for 3 seconds so you can focus another app like TextEdit ...");
setTimeout(() => run(), 3000);
