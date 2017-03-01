const MPEGTSHeader   = 0 // Symbol('MPEGTSHeader')
const MPEGTSAdaption = 1 // Symbol('MPEGTSAdaption')
const MPEGTSPES      = 2 // Symbol('MPEGTSPES')
const MPEGTSPSIPAT   = 3 // Symbol('MPEGTSPSIPAT')
const MPEGTSPSIPMT   = 4 // Symbol('MPEGTSPSIPMT')
const MPEGTSPSISDT   = 5 // Symbol('MPEGTSPSISDT')

const H264SPS      = 10 // Symbol('H264SPS')
const H264PPS      = 11 // Symbol('H264PPS')
const H264AUD      = 12 // Symbol('H264AUD')
const H264SEI      = 13 // Symbol('H264SEI')
const H264SliceIDR = 14 // Symbol('H264SliceIDR')

function parse(v) {
  switch (v) {
  case 0:
    return MPEGTSHeader
  case 1:
    return MPEGTSAdaption
  case 2:
    return MPEGTSPES
  case 3:
    return MPEGTSPSIPAT
  case 4:
    return MPEGTSPSIPMT
  case 5:
    return MPEGTSPSISDT

  case 6:
    return H264SPS
  case 7:
    return H264PPS
  case 8:
    return H264AUD
  case 9:
    return H264SEI
  case 10:
    return H264SliceIDR
  }

  return MPEGTSHeader
}

function toString(v) {
  switch (v) {
  case MPEGTSHeader:
    return 'mpegts-header'
  case MPEGTSAdaption:
    return 'mpegts-adaption'
  case MPEGTSPES:
    return 'mpegts-pes'
  case MPEGTSPSIPAT:
    return 'mpegts-psi-pat'
  case MPEGTSPSIPMT:
    return 'mpegts-psi-pmt'
  case MPEGTSPSISDT:
    return 'mpegts-psi-sdt'

  case H264SPS:
    return 'h264-sps'
  case H264PPS:
    return 'h264-pps'
  case H264AUD:
    return 'h264-aud'
  case H264SEI:
    return 'h264-sei'
  case H264SliceIDR:
    return 'h264-slice-idr'
  }

  return 'unknown'
}

function isMPEGTS(v) {
  if ((v == MPEGTSHeader) ||
      (v == MPEGTSAdaption) ||
      (v == MPEGTSPES) ||
      (v == MPEGTSPSIPAT) ||
      (v == MPEGTSPSIPMT) ||
      (v == MPEGTSPSISDT)) {
    return true
  }

  return false
}

function isH264(v) {
  if ((v == H264SPS) ||
      (v == H264PPS) ||
      (v == H264AUD) ||
      (v == H264SEI) ||
      (v == H264SliceIDR)) {
    return true
  }

  return false
}

export default {
  MPEGTSHeader,
  MPEGTSAdaption,
  MPEGTSPES,
  MPEGTSPSIPAT,
  MPEGTSPSIPMT,
  MPEGTSPSISDT,

  H264SPS,
  H264PPS,
  H264AUD,
  H264SEI,
  H264SliceIDR,

  parse,
  toString,
  isMPEGTS,
  isH264
}
