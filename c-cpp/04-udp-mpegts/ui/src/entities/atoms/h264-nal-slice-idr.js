import BaseModel from '../base-model'
import H264NALSliceType from '../../lib/h264-nal-slice-type'

class H264NALSliceIDR extends BaseModel {
  static fromMessagePack(msg) {
    const sliceTypeRaw = msg[1][0]
    const picParameterSetID = msg[3]
    const frameNum = msg[4]
    const picOrderCntLsb = msg[5]

    const sliceType = H264NALSliceType.parse(sliceTypeRaw)

    const model = new H264NALSliceIDR(
        sliceType
      , picParameterSetID
      , frameNum
      , picOrderCntLsb
    )

    return model
  }

  constructor(sliceType, picParameterSetID, frameNum, picOrderCntLsb) {
    super()

    this.sliceType = sliceType
    this.picParameterSetID = picParameterSetID
    this.frameNum = frameNum
    this.picOrderCntLsb = picOrderCntLsb
  }
}

export default H264NALSliceIDR
