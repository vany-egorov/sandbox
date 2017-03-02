const UNKNOWN = 0
const DEV = 1
const TEST = 2
const DEMO = 3
const PROD = 4


function parse(v) {
  let v = String(v)
  v = v.toLowerCase()
  v.replace("_", "-");

  switch (v) {
  case "1":
  case "d":
  case "dev":
  case "development":
    return DEV

  case "2":
  case "t":
  case "tst":
  case "test":
    return TEST

  case "3":
  case "demo":
    return DEMO

  case "p":
  case "prod":
  case "production":
    return PROD
  }

  return UNKNOWN
}

function is_dev() {

}

function is_prod() {

}

function is

module.exports = {
  UNKNOWN: UNKNOWN,
  DEV: DEV,
  TEST: TEST,
  DEMO: DEMO,
  PROD: PROD,
}
