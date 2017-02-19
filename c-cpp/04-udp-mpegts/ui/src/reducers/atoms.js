import {ATOMS_ADD} from '../actions/index.js'

function atoms(state = [], action) {
  switch (action.type) {
  case ATOMS_ADD:
    return [
      action.atom,
      ...state
    ].slice(0, 50)
  default:
    return state
  }
}

export default atoms
