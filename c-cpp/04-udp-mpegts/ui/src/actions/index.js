const ATOMS_ADD_SINGLE = Symbol("ATOMS:ADD:SINGLE")
const ATOMS_ADD_MULTI = Symbol("ATOMS:ADD:MULTI")

/* Action Creators */
function atomsAddSingle(atom) {
  return {type: ATOMS_ADD_SINGLE, atom: atom}
}

function atomsAddMulti(atoms) {
  return {type: ATOMS_ADD_MULTI, atoms: atoms}
}

export {
  ATOMS_ADD_SINGLE,
  ATOMS_ADD_MULTI,
  atomsAddSingle,
  atomsAddMulti
}
