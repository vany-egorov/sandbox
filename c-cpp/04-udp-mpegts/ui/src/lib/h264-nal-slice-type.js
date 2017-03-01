const P   = 0 // Symbol("P")
const B   = 1 // Symbol("B")
const I   = 2 // Symbol("I")
const SP  = 3 // Symbol("SP")
const SI  = 4 // Symbol("SI")
const P2  = 5 // Symbol("P2")
const B2  = 6 // Symbol("B2")
const I2  = 7 // Symbol("I2")
const SP2 = 8 // Symbol("SP2")
const SI2 = 8 // Symbol("SI2")

const name = {
  [P]: "P",
  [B]: "B",
  [I]: "I",
  [SP]: "SP",
  [SI]: "SI",
  [P2]: "P2",
  [B2]: "B2",
  [I2]: "I2",
  [SP2]: "SP2",
  [SI2]: "SI2"
}

const nameShort = {
  [P]: "P",
  [B]: "B",
  [I]: "I",
  [SP]: "P",
  [SI]: "I",
  [P2]: "P",
  [B2]: "B",
  [I2]: "I",
  [SP2]: "P",
  [SI2]: "I"
}

function parse(v) {
  switch (v) {
  case 0:
    return P
  case 1:
    return B
  case 2:
    return I
  case 3:
    return SP
  case 4:
    return SI
  case 5:
    return P2
  case 6:
    return B2
  case 7:
    return I2
  case 8:
    return SP2
  case 9:
    return SI2
  }

  return P
}

function isI(v) {
  if ((v == I) ||
      (v == SI) ||
      (v == I2) ||
      (v == SI2)) {
    return true
  }

  return false
}

function isP(v) {
  if ((v == P) ||
      (v == SP) ||
      (v == P2) ||
      (v == SP2)) {
    return true
  }

  return false
}

function isB(v) {
  if ((v == B) ||
      (v == B2)) {
    return true
  }

  return false
}

function toString(v) {
  return name[v]
}

function toStringShort(v) {
  return nameShort[v]
}

export default {
  P,
  B,
  I,
  SP,
  SI,
  P2,
  B2,
  I2,
  SP2,
  SI2,

  parse,
  isI,
  isP,
  isB,
  toString,
  toStringShort
}
