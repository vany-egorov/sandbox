import {combineReducers} from 'redux'
import {ADD} from '../actions/index.js'

function atoms(state = [], action) {
  switch (action.type) {
  case ADD:
    return [
      ...state,
      action.atom
    ]
  default:
    return state
  }
}

const app = combineReducers({
  atoms
})

export default app
