import _ from "lodash"
import BaseModel from "../base-model"
import ak from "../../lib/atom-kind"
import H264NALSPS from "./h264-nal-sps"
import H264NALPPS from "./h264-nal-pps"
import H264NALSliceIDR from "./h264-nal-slice-idr"

class AtomWrapper extends BaseModel {
  static fromMessagePack(msg) {
    const id = msg[0]
    const offset = msg[1]
    const atomKindRaw = msg[2][0]

    const atomKind = ak.parse(atomKindRaw)

    const model = new AtomWrapper(id, offset, atomKind)

    switch (model.atomKind) {
    case ak.H264SliceIDR:
      model.atom = H264NALSliceIDR.fromMessagePack(msg[3])
      break
    case ak.H264SPS:
      model.atom = H264NALSPS.fromMessagePack(msg[3])
      break
    case ak.H264PPS:
      model.atom = H264NALPPS.fromMessagePack(msg[3])
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

  normalized() {
    const result = {
      id: this.id,
      offset: this.offset,
      atomKind: this.atomKind
    }

    switch (this.atomKind) {
    case ak.H264SliceIDR:
      return _.extend(result, this.atom.normalized())
    case ak.H264SPS:
      return _.extend(result, this.atom.normalized())
    case ak.H264PPS:
      return _.extend(result, this.atom.normalized())
    default:
      return result
    }
  }
}

export default AtomWrapper
