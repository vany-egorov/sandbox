const dict = {
  P   : Symbol('P'),   // 0
  B   : Symbol('B'),   // 1
  I   : Symbol('I'),   // 2
  SP  : Symbol('SP'),  // 3
  SI  : Symbol('SI'),  // 4
  P2  : Symbol('P2'),  // 5
  B2  : Symbol('B2'),  // 6
  I2  : Symbol('I2'),  // 7
  SP2 : Symbol('SP2'), // 8
  SI2 : Symbol('SI2')  // 9
}

const name = {
  [dict.P]: 'P',
  [dict.B]: 'B',
  [dict.I]: 'I',
  [dict.SP]: 'SP',
  [dict.SI]: 'SI',
  [dict.P2]: 'P2',
  [dict.B2]: 'B2',
  [dict.I2]: 'I2',
  [dict.SP2]: 'SP2',
  [dict.SI2]: 'SI2'
}

const nameSimple = {
  [dict.P]: 'P',
  [dict.B]: 'B',
  [dict.I]: 'I',
  [dict.SP]: 'P',
  [dict.SI]: 'I',
  [dict.P2]: 'P',
  [dict.B2]: 'B',
  [dict.I2]: 'I',
  [dict.SP2]: 'P',
  [dict.SI2]: 'I'
}

function enumParse(v) {
  switch (v) {
  case 0:
    return dict.P
  case 1:
    return dict.B
  case 2:
    return dict.I
  case 3:
    return dict.SP
  case 4:
    return dict.SI
  case 5:
    return dict.P2
  case 6:
    return dict.B2
  case 7:
    return dict.I2
  case 8:
    return dict.SP2
  case 9:
    return dict.SI2
  }

  return dict.I
}

class H264NALSliceType {
  static parse(v) {
    return new H264NALSliceType(enumParse(v))
  }

  constructor(v) {
    this.v = v
  }

  toString() {
    return name[this.v]
  }

  toStringSimple() {
    return nameSimple[this.v]
  }
}

export default H264NALSliceType
