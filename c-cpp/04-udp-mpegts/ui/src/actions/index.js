const ATOMS_ADD = Symbol('ATOMS:ADD')

/* Action Creators */
function atomsAdd(atom) {
  return {type: ATOMS_ADD, atom: atom}
}

export {atomsAdd, ATOMS_ADD}
