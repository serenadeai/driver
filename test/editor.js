const driver = require("../index");

const run = async () => {
  console.log(await driver.getEditorState());
};

setInterval(() => {
  run();
}, 1000);
