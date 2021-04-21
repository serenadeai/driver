const driver = require("../index");

const run = async () => {
  setInterval(async () => {
    const state = await driver.getEditorState();
    const text = state.text;
    const cursor = state.cursor;
    console.log(state);
    console.log(text.substring(0, cursor) + "<>" + text.substring(cursor, text.length));
  }, 500);
};

run();
