import ak from '../lib/atom-kind'
import {ATOMS_ADD} from '../actions/index.js'

function atoms(state = [], action) {
  switch (action.type) {
  case ATOMS_ADD:
    if (action.atom.atomKind == ak.H264SPS ||
        action.atom.atomKind == ak.H264PPS ||
        action.atom.atomKind == ak.H264AUD ||
        action.atom.atomKind == ak.H264SEI ||
        action.atom.atomKind == ak.H264SliceIDR) {
      return [
        action.atom.normalized(),
        ...state
      ].slice(0, 50)
    }
    return state
  default:
    return state
  }
}

export default atoms
