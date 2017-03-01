import _ from "lodash"
import Bacon from "baconjs"
import {
  atomsAddSingle, atomsAddMulti,
  ATOMS_ADD_SINGLE, ATOMS_ADD_MULTI
} from "./actions"

class Store {
  constructor() {
    this.i = new Bacon.Bus()
    this.o = new Bacon.Bus()
    this.unsubs = _([])
  }

  /* i */
  dispatch(action) { this.i.push(action) }

  register(handler) {
    const unsub1 = this.i
      .filter((a) => { return a.type == ATOMS_ADD_SINGLE })
      .bufferWithTimeOrCount(500, 3)
      .onValue((atoms) => { this.dispatch(atomsAddMulti(atoms)) })

    const unsub2 = this.i
      .doAction(() => { /* middleware */ })
      .onValue(handler)

    this.unsubs
      .push(unsub1)
      .push(unsub2)
      .commit()
  }

  /* o */
  emit(action) { this.o.push(action) }

  on(actionType, callback) {
    return this.o
      .filter((a) => { return a.type == actionType })
      .onValue(callback)
  }

  destructor() {
    this.unsubs
      .forEach((u) => { u() })
  }
}

function main() {
  const store = new Store()

  store.register((action) => {
    switch (action.type) {
    case ATOMS_ADD_MULTI:
      store.emit(action)
      break
    }
  })
  store.register((action) => {
    switch (action.type) {
    case ATOMS_ADD_MULTI:
      store.emit(action)
      break
    }
  })

  const atom = {foo: "bar"}

  const unsub1 = store.on(ATOMS_ADD_MULTI, () => {
    console.log("[<~~~] add-multi-1")
  })
  const unsub2 = store.on(ATOMS_ADD_MULTI, () => {
    console.log("[<~~~] add-multi-2")
  })

  store.destructor()

  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))

  unsub1()
  unsub2()
  console.log("unsub")

  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
  store.dispatch(atomsAddSingle(atom))
}

main()
