import BaseModel from '../base-model'
import H264NALSliceType from '../../lib/h264-nal-slice-type'

class H264NALSliceIDR extends BaseModel {
  static fromMessagePack(msg) {
    const size = msg[1]
    const sliceTypeRaw = msg[2][0]
    const picParameterSetID = msg[4]
    const frameNum = msg[5]
    const picOrderCntLsb = msg[6]

    const sliceType = H264NALSliceType.parse(sliceTypeRaw)

    const model = new H264NALSliceIDR(
        size
      , sliceType
      , picParameterSetID
      , frameNum
      , picOrderCntLsb
    )

    return model
  }

  constructor(size, sliceType, picParameterSetID, frameNum, picOrderCntLsb) {
    super()

    this.size = size
    this.sliceType = sliceType
    this.picParameterSetID = picParameterSetID
    this.frameNum = frameNum
    this.picOrderCntLsb = picOrderCntLsb
  }

  normalized() {
    return {
      size: this.size,
      sliceType: this.sliceType,
      picParameterSetID: this.picParameterSetID,
      frameNum: this.frameNum,
      picOrderCntLsb: this.picOrderCntLsb
    }
  }
}

export default H264NALSliceIDR
