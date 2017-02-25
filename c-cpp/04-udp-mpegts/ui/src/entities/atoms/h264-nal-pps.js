import H264NAL from './h264-nal'

class H264NALPPS extends H264NAL {
  static fromMessagePack(msg) {
    const model = new H264NALPPS()

    model.fromMessagePack(msg)

    return model
  }

  constructor() {
    super()
  }

  normalized() {
    return {
      sz: this.sz
    }
  }
}

export default H264NALPPS
