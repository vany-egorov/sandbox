import BaseModel from '../base-model'

class H264NALSPS extends BaseModel {
  static fromMessagePack(msg) {
    const size = msg[1]

    const model = new H264NALSPS(size)

    return model
  }

  constructor(size) {
    super()

    this.size = size
  }

  normalized() {
    return {
      size: this.size
    }
  }
}

export default H264NALSPS
