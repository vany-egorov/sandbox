const {log} = require("../common/log")

function task() {
  return new Promise((resolve, reject) => {
    log("[task] cp")
    resolve()
  })
}

module.exports = task
