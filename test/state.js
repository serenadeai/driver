const driver = require("../index");

const run = async () => {
  setInterval(async () => {
    const state = await driver.getEditorState();
    console.log(state);
  }, 500);
};

run();
