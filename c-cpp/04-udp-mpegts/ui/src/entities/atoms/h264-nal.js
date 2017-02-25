import BaseModel from '../base-model'
import h264NALType from '../../lib/h264-nal-type'

class H264NAL extends BaseModel {
  fromMessagePack(msg) {
    this.nt = h264NALType.parse(msg[0][0])
    this.sz = msg[1]
  }
}

export default H264NAL
