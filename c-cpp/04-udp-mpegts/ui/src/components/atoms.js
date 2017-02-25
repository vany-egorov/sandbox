import React from 'react'
import ak from '../lib/atom-kind'
import h264NALSliceType from '../lib/h264-nal-slice-type'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    switch (model.atomKind) {
    case ak.H264SliceIDR:
      return (
        <div className="atoms-grid__row">
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {h264NALSliceType.toString(model.sliceType)} slice #0 (
            frame-num: {model.frameNum},
            pic-order-cnt-lsb: {model.picOrderCntLsb}
          )
        </div>
      )
    default:
      return (
        <div className="atoms-grid__row">
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    }
  }
}

class Atoms extends React.Component {
  createChildComponent(model) {
    const key = model.id
    return <Atom model={model} key={key}/>
  }

  render() {
    const collection = this.props.collection
    const items = collection.map(this.createChildComponent)

    return (
      <div className="atoms-grid">
        {items}
      </div>
    )
  }
}

export default Atoms
