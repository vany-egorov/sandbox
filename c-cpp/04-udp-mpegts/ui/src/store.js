import _ from "lodash"
import Bacon from "baconjs"
import {atomsAddMulti,
        ATOMS_ADD_SINGLE, ATOMS_ADD_MULTI} from "./actions"

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
      .map((a) => { return a.atom })
      .bufferWithTimeOrCount(500, 10)
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

    this.i.end()
    this.o.end()
  }
}

const store = new Store()
store.register((action) => {
  switch (action.type) {
  case ATOMS_ADD_MULTI:
    store.emit(action)
    break
  }
})

export default store
