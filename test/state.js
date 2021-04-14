const driver = require("../index");

const run = async () => {
  setInterval(async () => {
    const state = await driver.getEditorState();
    const text = state.text;
    const cursor = state.cursor;
    const axLoc = state.axlocation;
    console.log(state);
    console.log(
      text.substring(0, axLoc) +
        "|" +
        text.substring(axLoc, cursor) +
        "<>" +
        text.substring(cursor, text.length)
    );
  }, 500);
};

run();
