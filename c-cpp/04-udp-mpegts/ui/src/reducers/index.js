import {combineReducers} from 'redux'
import {ADD} from '../actions/index.js'

let counter = 1

function reducer(state = {}, action) {
  console.log(state, action)

  switch (action.type) {
  case ADD:
    console.log('add action passed to reducer')
    return Object.assign({}, state, {
      counter: counter++
    })
  default:
    console.log('unknown action passed to reducer')
    return state
  }
}

const app = combineReducers({
  reducer
})

export default app
