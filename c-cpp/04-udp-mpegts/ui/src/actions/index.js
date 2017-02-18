const ADD = Symbol('ADD')

/* Action Creators */
function add(atom) {
  console.log('add atom action')
  return {type: ADD, atom: atom}
}

export {ADD, add}
