import _ from "lodash"
import ak from "../lib/atom-kind"
import {ATOMS_ADD_SINGLE, ATOMS_ADD_MULTI} from "../actions/index.js"

// let isDumped = false

function atoms(state = [], action) {
  let stateNew = []

  // if ((state.length == 300) && (!isDumped)) {
  //   console.log(JSON.stringify(state))
  //   isDumped = true
  // }

  switch (action.type) {
  case ATOMS_ADD_SINGLE:
    if (// action.atom.atomKind == ak.H264SPS ||
        // action.atom.atomKind == ak.H264PPS ||
        // action.atom.atomKind == ak.H264AUD ||
        // action.atom.atomKind == ak.H264SEI ||
        action.atom.atomKind == ak.H264SliceIDR) {
      stateNew = [
        action.atom.normalized(),
        ...state
      ].slice(0, 50)

      return stateNew
    }

    return state

  case ATOMS_ADD_MULTI:
    return _(action.atoms)
      .filter((atom) => {
        return (atom.atomKind == ak.H264SPS ||
                atom.atomKind == ak.H264PPS ||
                atom.atomKind == ak.H264AUD ||
                atom.atomKind == ak.H264SEI ||
                atom.atomKind == ak.H264SliceIDR)
      })
      .map((atom) => {
        return atom.normalized()
      })
      .reverse()
      .value()
      .concat(state)
      .slice(0, 50)
  default:
    return state
  }
}

export default atoms
