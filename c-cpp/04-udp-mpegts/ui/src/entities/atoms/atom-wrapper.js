import BaseModel from '../base-model'
import ak from '../../lib/atom-kind'
import H264NALSliceIDR from './h264-nal-slice-idr'

class AtomWrapper extends BaseModel {
  static fromMessagePack(msg) {
    const id = msg[0]
    const offset = msg[1]
    const atomKindRaw = msg[2][0]

    const atomKind = ak.AtomKind.parse(atomKindRaw)

    const model = new AtomWrapper(id, offset, atomKind)

    switch (model.atomKind.v) {
    case ak.H264SliceIDR:
      model.atom = H264NALSliceIDR.fromMessagePack(msg[3])
      break
    }

    return model
  }

  constructor(id, offset, atomKind) {
    super()

    this.id = id
    this.offset = offset
    this.atomKind = atomKind
  }
}

export default AtomWrapper
