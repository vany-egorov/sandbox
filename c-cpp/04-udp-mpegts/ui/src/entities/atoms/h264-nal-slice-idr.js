import H264NAL from "./h264-nal"
import h264NALSliceType from "../../lib/h264-nal-slice-type"

class H264NALSliceIDR extends H264NAL {
  static fromMessagePack(msg) {
    const sliceTypeRaw = msg[2][0]
    const picParameterSetID = msg[4]
    const frameNum = msg[5]
    const picOrderCntLsb = msg[6]

    const sliceType = h264NALSliceType.parse(sliceTypeRaw)

    const model = new H264NALSliceIDR(
        sliceType
      , picParameterSetID
      , frameNum
      , picOrderCntLsb
    )
    model.fromMessagePack(msg)

    return model
  }

  constructor(sliceType, picParameterSetID, frameNum, picOrderCntLsb) {
    super()

    this.sliceType = sliceType
    this.picParameterSetID = picParameterSetID
    this.frameNum = frameNum
    this.picOrderCntLsb = picOrderCntLsb
  }

  normalized() {
    return {
      sz: this.sz,
      sliceType: this.sliceType,
      picParameterSetID: this.picParameterSetID,
      frameNum: this.frameNum,
      picOrderCntLsb: this.picOrderCntLsb
    }
  }
}

export default H264NALSliceIDR
