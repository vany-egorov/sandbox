const Unspecified = 0
const Slice       = 1
const DPA         = 2
const DPB         = 3
const DPC         = 4
const IDR         = 5
const SEI         = 6
const SPS         = 7
const PPS         = 8
const AUD         = 9
const EOSEQ       = 10
const EOSTREAM    = 11
const FILL        = 12
const SPSEXT      = 13
const Prefix      = 14
const SSPS        = 15
const DPS         = 16
const CSOACPWP    = 19
const CSE         = 20
const CSE3D       = 21

function parse(v) {
  switch (v) {
  case 0:
    return Unspecified
  case 1:
    return Slice
  case 2:
    return DPA
  case 3:
    return DPB
  case 4:
    return DPC
  case 5:
    return IDR
  case 6:
    return SEI
  case 7:
    return SPS
  case 8:
    return PPS
  case 9:
    return AUD
  case 10:
    return EOSEQ
  case 11:
    return EOSTREAM
  case 12:
    return FILL
  case 13:
    return SPSEXT
  case 14:
    return Prefix
  case 15:
    return SSPS
  case 16:
    return DPS
  case 19:
    return CSOACPWP
  case 20:
    return CSE
  case 21:
    return CSE3D
  }

  return Unspecified
}

function toString(v) {
  switch (v) {
  case Unspecified:
    return 'Unspecified'
  case Slice:
    return 'Slice'
  case DPA:
    return 'DPA'
  case DPB:
    return 'DPB'
  case DPC:
    return 'DPC'
  case IDR:
    return 'IDR'
  case SEI:
    return 'SEI'
  case SPS:
    return 'SPS'
  case PPS:
    return 'PPS'
  case AUD:
    return 'AUD'
  case EOSEQ:
    return 'EOSEQ'
  case EOSTREAM:
    return 'EOSTREAM'
  case FILL:
    return 'FILL'
  case SPSEXT:
    return 'SPSEXT'
  case Prefix:
    return 'Prefix'
  case SSPS:
    return 'SSPS'
  case DPS:
    return 'DPS'
  case CSOACPWP:
    return 'CSOACPWP'
  case CSE:
    return 'CSE'
  case CSE3D:
    return 'CSE3D'
  }

  return 'Unspecified'
}

export default {
  Unspecified,
  Slice,
  DPA,
  DPB,
  DPC,
  IDR,
  SEI,
  SPS,
  PPS,
  AUD,
  EOSEQ,
  EOSTREAM,
  FILL,
  SPSEXT,
  Prefix,
  SSPS,
  DPS,
  CSOACPWP,
  CSE,
  CSE3D,

  parse,
  toString
}
